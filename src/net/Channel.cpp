#include "Channel.h"
#include <sys/epoll.h>
#include <unistd.h>

// 定义epoll事件常量
// kNoneEvent: 不关注任何事件
const int Channel::kNoneEvent = 0;
// kReadEvent: 可读事件（EPOLLIN是普通数据，EPOLLPRI是紧急数据）
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
// kWriteEvent: 可写事件
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(int fd)
    : fd_(fd),
      events_(0),
      revents_(0) {
}

Channel::~Channel() {
    // 重要：Channel不负责关闭fd！
    // 因为fd的所有权不在Channel
    // fd可能属于Socket类或其他类
}

void Channel::handleEvent() {
    // 这是Channel的核心函数！
    // 当epoll_wait返回时，EventLoop会调用这个函数
    // 根据revents_（实际发生的事件）调用相应的回调
    
    // EPOLLHUP: 对端关闭连接
    // 注意：关闭事件时，可能还有数据可读，所以不能直接return
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        // 对端关闭，且没有数据可读
        if (closeCallback_) {
            closeCallback_();
        }
    }
    
    // EPOLLERR: 发生错误
    if (revents_ & EPOLLERR) {
        if (errorCallback_) {
            errorCallback_();
        }
    }
    
    // EPOLLIN或EPOLLPRI: 有数据可读
    // EPOLLRDHUP: 对端关闭了写端（半关闭）
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readCallback_) {
            readCallback_();
        }
    }
    
    // EPOLLOUT: 可以写数据了
    if (revents_ & EPOLLOUT) {
        if (writeCallback_) {
            writeCallback_();
        }
    }
}