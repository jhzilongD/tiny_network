#include "EventLoopThread.h"
#include "EventLoop.h"
#include <iostream>

EventLoopThread::EventLoopThread(const std::string& name)
    : loop_(nullptr),
      name_(name)
{
    std::cout << "EventLoopThread[" << name_ << "] created" << std::endl;
}

EventLoopThread::~EventLoopThread() {
    if (loop_ != nullptr) {
        // 关闭EventLoop
        loop_->quit();
        // 等待线程结束
        if (thread_.joinable()) {
            thread_.join();
        }
    }
    std::cout << "EventLoopThread[" << name_ << "] destroyed" << std::endl;
}

// 启动线程
EventLoop* EventLoopThread::startLoop() {
    // 启动线程，运行threadFunc
    thread_ = std::thread(&EventLoopThread::threadFunc, this);
    
    EventLoop* loop = nullptr;
    {// 这里目前不是很懂！！
        // 等待新线程创建EventLoop
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    
    std::cout << "EventLoopThread[" << name_ << "] started, loop=" << loop << std::endl;
    return loop;
}

// 线程函数
void EventLoopThread::threadFunc() {
    std::cout << "EventLoopThread[" << name_ << "] thread started" << std::endl;
    
    // 在新线程中创建EventLoop
    EventLoop loop;
    
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        // 通知startLoop()函数EventLoop已创建
        cond_.notify_one();
    }
    
    // 运行事件循环
    loop.loop();
    
    // loop结束
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
    std::cout << "EventLoopThread[" << name_ << "] thread exiting" << std::endl;
}