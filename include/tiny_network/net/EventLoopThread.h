#ifndef TINY_NETWORK_NET_EVENTLOOPTHREAD_H
#define TINY_NETWORK_NET_EVENTLOOPTHREAD_H

#include "../base/noncopyable.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

class EventLoop;

// EventLoopThread：在独立线程中运行EventLoop
// 
// 使用方式：在主线程里
// EventLoopThread loopThread;
// EventLoop* loop = loopThread.startLoop();
// loop就运行在新线程中了！
class EventLoopThread : noncopyable {
public:
    EventLoopThread(const std::string& name = "EventLoopThread");
    ~EventLoopThread();
    
    // 启动线程，返回新线程中的EventLoop对象
    EventLoop* startLoop();
    
private:
    // 线程函数
    void threadFunc();
    
    EventLoop* loop_;                      // EventLoop对象
    std::thread thread_;                   // 线程对象
    std::mutex mutex_;                     // 保护loop_
    std::condition_variable cond_;         // 等待EventLoop创建完成
    std::string name_;                      // 线程名称
};

#endif