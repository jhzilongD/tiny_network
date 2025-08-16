#ifndef TINY_NETWORK_LOGGER_ASYNCLOGGING_H
#define TINY_NETWORK_LOGGER_ASYNCLOGGING_H

#include "../base/noncopyable.h"
#include "../base/Thread.h"
#include "LogStream.h"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>

class LogFile;

// AsyncLogging：异步日志系统的核心
//
// 架构：
//   前端线程 → append() → 写入缓冲区 → 通知后端
//   后端线程 → 获取满缓冲区 → 写入LogFile → 返回空缓冲区
//
// 双缓冲技术：
//   currentBuffer_：前端正在写入
//   nextBuffer_：备用缓冲区
//   buffers_：满缓冲区队列，等待后端处理
class AsyncLogging : noncopyable {
public:
    AsyncLogging(const std::string& basename, size_t rollSize = 500*1000*1000);
    ~AsyncLogging();

    // 启动/停止异步日志
    void start();
    void stop();

    // 前端接口：追加日志（高性能，几乎无阻塞）
    void append(const char* logline, int len);

private:
    // 后台线程函数
    void threadFunc();

    // 缓冲区类型定义
    using LargeBuffer = LogStream::FixedBuffer<4000*1000>;  // 4MB大缓冲区
    using LargeBufferPtr = std::unique_ptr<LargeBuffer>;
    using BufferVector = std::vector<LargeBufferPtr>;

    const std::string basename_;    // 日志文件名前缀
    const size_t rollSize_;         // 文件滚动大小
    
    bool running_;                  // 运行状态
    Thread thread_;                 // 后台线程
    
    // 双缓冲区 + 队列（需要锁保护）
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    
    LargeBufferPtr currentBuffer_;  // 当前写入缓冲区
    LargeBufferPtr nextBuffer_;     // 备用缓冲区
    BufferVector buffers_;          // 满缓冲区队列
};

#endif