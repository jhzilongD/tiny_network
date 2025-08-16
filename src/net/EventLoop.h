#ifndef TINY_NETWORK_NET_EVENTLOOP_H
#define TINY_NETWORK_NET_EVENTLOOP_H

#include "../base/noncopyable.h"
#include "../base/CurrentThread.h"
#include <memory>
#include <vector>
#include <atomic>
#include <functional>
#include <mutex>

class Channel;
class Poller;

// EventLoop：事件循环（Reactor模式的核心）
// 
// 职责：
// 1. 调用Poller::poll()等待事件
// 2. 分发事件给活跃的Channel
// 3. 执行用户的回调任务
//
// 重要原则：One Loop Per Thread
// 一个线程最多只能有一个EventLoop
class EventLoop : noncopyable {
public:
    using Functor = std::function<void()>;
    
    EventLoop();
    ~EventLoop();
    
    // 开始事件循环
    // 这个函数会一直运行，直到quit()被调用
    void loop();
    
    // 退出事件循环
    void quit();
    
    // 在EventLoop线程中执行回调
    void runInLoop(Functor cb);
    
    // 把回调放入队列，在EventLoop线程中执行
    void queueInLoop(Functor cb);
    
    // 唤醒EventLoop线程
    void wakeup();
    
    // 更新Channel（其实是转发给Poller）
    void updateChannel(Channel* channel);
    
    // 移除Channel
    void removeChannel(Channel* channel);
    
    // 判断是否在EventLoop线程中
    bool isInLoopThread() const {
        return threadId_ == CurrentThread::tid();
    }
    
    // 判断是否在循环中
    bool isLooping() const { return looping_; }

private:
    using ChannelList = std::vector<Channel*>;
    
    // 处理eventfd的读事件
    void handleRead();
    
    // 执行待处理的回调函数
    void doPendingFunctors();
    
    bool looping_;                     // 是否正在循环
    std::atomic<bool> quit_;           // 是否要退出循环
    bool callingPendingFunctors_;      // 是否正在执行待处理的回调
    
    const pid_t threadId_;              // 创建EventLoop的线程ID
    std::unique_ptr<Poller> poller_;   // Poller对象（用unique_ptr自动管理）
    
    int wakeupFd_;                      // eventfd，用于唤醒EventLoop
    std::unique_ptr<Channel> wakeupChannel_; // 监听wakeupFd_的Channel
    
    ChannelList activeChannels_;       // 活跃的Channel列表
    
    std::mutex mutex_;                  // 保护pendingFunctors_
    std::vector<Functor> pendingFunctors_; // 待执行的回调函数
};

#endif