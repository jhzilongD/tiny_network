#include "AsyncLogging.h"
#include "LogFile.h"
#include <assert.h>
#include <iostream>
#include <chrono>

AsyncLogging::AsyncLogging(const std::string& basename, size_t rollSize)
    : basename_(basename),
      rollSize_(rollSize),
      running_(false),
      thread_(std::bind(&AsyncLogging::threadFunc, this), "AsyncLogging"),
      currentBuffer_(new LargeBuffer),
      nextBuffer_(new LargeBuffer)
{
    // 预分配两个缓冲区
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);  // 预留16个缓冲区空间
}

AsyncLogging::~AsyncLogging() {
    if (running_) {
        stop();
    }
}

void AsyncLogging::start() {
    running_ = true;
    thread_.start();
}

void AsyncLogging::stop() {
    running_ = false;
    cond_.notify_one();  // 通知后台线程退出
    thread_.join();
}

// 前端接口：追加日志（关键函数）
void AsyncLogging::append(const char* logline, int len) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查当前缓冲区是否有足够空间
    if (currentBuffer_->avail() > len) {
        // 有空间，直接追加（常见情况，很快）
        currentBuffer_->append(logline, len);
    } else {
        // 空间不够，需要换缓冲区
        
        // 1. 把满的currentBuffer_放入队列
        buffers_.push_back(std::move(currentBuffer_));
        
        // 2. 尝试使用nextBuffer_作为新的currentBuffer_
        if (nextBuffer_) {
            currentBuffer_ = std::move(nextBuffer_);
        } else {
            // nextBuffer_也没有，分配新的（不常见）
            currentBuffer_.reset(new LargeBuffer);
        }
        
        // 3. 追加到新缓冲区
        currentBuffer_->append(logline, len);
        
        // 4. 通知后台线程处理
        cond_.notify_one();
    }
}

// 后台线程函数（核心逻辑）
void AsyncLogging::threadFunc() {
    assert(running_);
    
    // 创建LogFile
    LogFile output(basename_, rollSize_, false);  // 后台线程专用，无需线程安全
    
    // 准备两个空缓冲区供前端使用
    LargeBufferPtr newBuffer1(new LargeBuffer);
    LargeBufferPtr newBuffer2(new LargeBuffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    
    BufferVector buffersToWrite;  // 本轮要写入的缓冲区
    buffersToWrite.reserve(16);
    
    while (running_) {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());
        
        // 1. 临界区：获取满缓冲区
        {
            std::unique_lock<std::mutex> lock(mutex_);
            
            if (buffers_.empty()) {  // 没有满缓冲区
                // 等待3秒或者被通知
                cond_.wait_for(lock, std::chrono::seconds(3));
            }
            
            // 无论如何，都把currentBuffer_交换出来
            // （避免日志延迟太久）
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            
            // 交换满缓冲区到本地
            buffersToWrite.swap(buffers_);
            
            // 确保有nextBuffer_
            if (!nextBuffer_) {
                nextBuffer_ = std::move(newBuffer2);
            }
        }
        
        assert(!buffersToWrite.empty());
        
        // 2. 写入文件（在临界区外，不阻塞前端）
        if (buffersToWrite.size() > 25) {
            // 缓冲区太多，丢弃一些（避免内存爆炸）
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages, %zd larger buffers\n",
                     buffersToWrite.size()-2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            
            // 只保留前两个缓冲区
            buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
        }
        
        // 逐个写入缓冲区到文件
        for (const auto& buffer : buffersToWrite) {
            output.append(buffer->data(), buffer->length());
        }
        
        // 3. 回收缓冲区
        if (buffersToWrite.size() > 2) {
            // 多余的缓冲区丢弃（避免内存过多）
            buffersToWrite.resize(2);
        }
        
        if (!newBuffer1) {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }
        
        if (!newBuffer2) {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        
        buffersToWrite.clear();
        output.flush();  // 刷新到磁盘
    }
    
    output.flush();  // 最后刷新
}