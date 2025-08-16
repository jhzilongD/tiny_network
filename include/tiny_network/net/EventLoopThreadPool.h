#ifndef TINY_NETWORK_NET_EVENTLOOPTHREADPOOL_H
#define TINY_NETWORK_NET_EVENTLOOPTHREADPOOL_H

#include "../base/noncopyable.h"
#include <vector>
#include <memory>
#include <string>

class EventLoop;
class EventLoopThread;

// EventLoopThreadPool：EventLoop线程池
// 
// 管理多个EventLoopThread，实现负载均衡
// 主要用于TcpServer，把新连接分配给不同的EventLoop
class EventLoopThreadPool : noncopyable {
public:
    EventLoopThreadPool(EventLoop* baseLoop, const std::string& name);
    ~EventLoopThreadPool();
    
    // 设置线程数量
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    
    // 启动线程池
    void start();
    
    // 获取下一个EventLoop（轮询方式）
    EventLoop* getNextLoop();
    
    // 获取所有EventLoop
    std::vector<EventLoop*> getAllLoops();
    
    bool started() const { return started_; }
    const std::string& name() const { return name_; }

private:
    EventLoop* baseLoop_;                                    // 主线程的EventLoop
    std::string name_;                                       // 线程池名称
    bool started_;                                           // 是否已启动
    int numThreads_;                                         // 线程数量
    int next_;                                               // 轮询索引
    std::vector<std::unique_ptr<EventLoopThread>> threads_;  // 线程列表
    std::vector<EventLoop*> loops_;                          // EventLoop列表
};

#endif