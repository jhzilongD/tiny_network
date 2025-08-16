#ifndef TINY_NETWORK_NET_CHANNEL_H
#define TINY_NETWORK_NET_CHANNEL_H

#include <functional>
#include <sys/epoll.h>
#include "noncopyable.h"

class Channel : private noncopyable {
public:
    // 事件回调函数类型
    using EventCallback = std::function<void()>;
    using UpdateCallback = std::function<void(Channel*)>;  // 更新回调：通知外部注册/修改事件
    
    Channel(int fd);  // 构造时传入负责的文件描述符
    ~Channel();
    
    // 处理事件（这是最重要的函数！）
    void handleEvent();
    
    // 设置回调函数
    void setReadCallback(EventCallback cb) { readCallback_ = cb; }
    void setWriteCallback(EventCallback cb) { writeCallback_ = cb; }
    void setErrorCallback(EventCallback cb) { errorCallback_ = cb; }
    void setCloseCallback(EventCallback cb) { closeCallback_ = cb; }
    
    // 获取文件描述符
    int fd() const { return fd_; }
    
    // 获取感兴趣的事件
    int events() const { return events_; }
    
    // 设置实际发生的事件（epoll会调用这个）
    void set_revents(int revents) { revents_ = revents; }
    
    // 设置感兴趣的事件
    void enableReading() { events_ |= kReadEvent; }
    void enableWriting() { events_ |= kWriteEvent; }
    void disableWriting() { events_ &= ~kWriteEvent; }
    void disableAll() { events_ = kNoneEvent; }
    
    // 判断是否在监听写事件
    bool isWriting() const { return events_ & kWriteEvent; }

private:
    // epoll事件常量
    static const int kNoneEvent;
    static const int kReadEvent; 
    static const int kWriteEvent;
    
    const int fd_;    // 负责的文件描述符（不会改变，所以用const）
    int events_;      // 感兴趣的事件（我们要监听什么事件，可能有多个感兴趣的
    int revents_;     // 实际发生的事件（epoll告诉我们发生了什么事件）
    
    // 各种事件的回调函数
    EventCallback readCallback_;   // 可读时调用
    EventCallback writeCallback_;  // 可写时调用
    EventCallback errorCallback_;  // 出错时调用
    EventCallback closeCallback_;  // 连接关闭时调用
};

#endif