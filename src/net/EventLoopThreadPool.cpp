#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"
#include <iostream>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& name)
    : baseLoop_(baseLoop),
      name_(name),
      started_(false),
      numThreads_(0),
      next_(0)
{
    std::cout << "EventLoopThreadPool[" << name_ << "] created" << std::endl;
}

EventLoopThreadPool::~EventLoopThreadPool() {
    // EventLoopThread的析构函数会停止线程
    std::cout << "EventLoopThreadPool[" << name_ << "] destroyed" << std::endl;
}

void EventLoopThreadPool::start() {
    started_ = true;
    
    // 创建numThreads_个线程
    for (int i = 0; i < numThreads_; ++i) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        
        // 创建EventLoopThread
        auto t = std::make_unique<EventLoopThread>(buf);
        
        // 启动线程，获取EventLoop
        loops_.push_back(t->startLoop());
        
        // 保存EventLoopThread
        threads_.push_back(std::move(t));
    }
    
    // 如果numThreads_为0，说明是单线程模式
    if (numThreads_ == 0) {
        std::cout << "EventLoopThreadPool[" << name_ << "] single thread mode" << std::endl;
    } else {
        std::cout << "EventLoopThreadPool[" << name_ << "] started with " 
                  << numThreads_ << " threads" << std::endl;
    }
}

// 轮询（Round-Robin）方式获取下一个EventLoop
EventLoop* EventLoopThreadPool::getNextLoop() {
    EventLoop* loop = baseLoop_;  // 默认返回主线程的EventLoop
    
    // 如果有IO线程，使用轮询
    if (!loops_.empty()) {
        // 轮询
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    if (loops_.empty()) {
        return std::vector<EventLoop*>(1, baseLoop_);
    } else {
        return loops_;
    }
}