#include "Poller.h"
#include "Channel.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <cassert>

// 构造函数：创建epoll实例
Poller::Poller() {
    // epoll_create1(0)创建一个epoll实例
    // 返回值是epoll的文件描述符
    epollfd_ = epoll_create1(0);
    if (epollfd_ < 0) {
        std::cerr << "Poller::Poller() epoll_create1 failed" << std::endl;
    }
}

// 析构函数：关闭epoll文件描述符
Poller::~Poller() {
    close(epollfd_);
}

// poll函数：等待事件发生（这是最重要的函数！）
void Poller::poll(int timeoutMs, ChannelList* activeChannels) {
    // 清空activeChannels，准备填充新的活跃Channel
    activeChannels->clear();
    
    // 准备一个数组来接收epoll_wait的结果
    // 这个数组的大小决定了一次最多能处理多少个事件
    struct epoll_event events[128];
    
    // 调用epoll_wait等待事件
    // 参数说明：
    // - epollfd_: epoll实例
    // - events: 用来接收发生的事件
    // - 128: events数组的大小
    // - timeoutMs: 超时时间（-1表示永远等待）
    int numEvents = epoll_wait(epollfd_, events, 128, timeoutMs);
    
    if (numEvents > 0) {
        std::cout << "Poller::poll() " << numEvents << " events happened" << std::endl;
        
        // 遍历所有发生的事件
        for (int i = 0; i < numEvents; ++i) {
            // epoll_event.data.ptr存储的是Channel指针
            // 我们在updateChannel时会设置这个指针
            Channel* channel = static_cast<Channel*>(events[i].data.ptr);
            
            // 设置Channel实际发生的事件
            channel->set_revents(events[i].events);
            
            // 把这个Channel加入到活跃列表
            activeChannels->push_back(channel);
        }
    } else if (numEvents == 0) {
        std::cout << "Poller::poll() timeout" << std::endl;
    } else {
        std::cerr << "Poller::poll() error" << std::endl;
    }
}

// updateChannel：添加或修改Channel
void Poller::updateChannel(Channel* channel) {
    int fd = channel->fd();
    
    // 查看这个fd是否已经在channels_中
    auto it = channels_.find(fd);
    
    if (it == channels_.end()) {
        // 新的Channel，需要添加到epoll
        std::cout << "Poller::updateChannel() ADD fd=" << fd << std::endl;
        
        // 保存到map中
        channels_[fd] = channel;
        
        // 准备epoll_event结构体
        struct epoll_event event;
        memset(&event, 0, sizeof(event));
        event.events = channel->events();  // Channel关注的事件
        event.data.ptr = channel;          // 存储Channel指针，方便后续使用
        
        // 添加到epoll
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event) < 0) {
            std::cerr << "Poller::updateChannel() EPOLL_CTL_ADD failed" << std::endl;
        }
    } else {
        // 已存在的Channel，修改它关注的事件
        std::cout << "Poller::updateChannel() MOD fd=" << fd << std::endl;
        
        // 更新map中的指针（虽然通常不会变）
        channels_[fd] = channel;
        
        // 准备新的event
        struct epoll_event event;
        memset(&event, 0, sizeof(event));
        event.events = channel->events();
        event.data.ptr = channel;
        
        // 修改epoll中的事件
        if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event) < 0) {
            std::cerr << "Poller::updateChannel() EPOLL_CTL_MOD failed" << std::endl;
        }
    }
}

// 从epoll中移除Channel
void Poller::removeChannel(Channel* channel) {
    int fd = channel->fd();
    
    // 确保Channel存在
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    
    std::cout << "Poller::removeChannel() DEL fd=" << fd << std::endl;
    
    // 从epoll中删除
    if (::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        std::cerr << "epoll_ctl DEL error: " << strerror(errno) << std::endl;
    }
    
    // 从map中删除
    channels_.erase(fd);
}