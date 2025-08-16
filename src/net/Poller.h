#ifndef TINY_NETWORK_NET_POLLER_H
#define TINY_NETWORK_NET_POLLER_H

#include "../base/noncopyable.h"
#include <vector>
#include <map>

// 前向声明，避免头文件循环依赖
class Channel;

// Poller类：把epoll封装成易用的C++接口
// 
// 使用方式：
// 1. 创建Poller对象
// 2. 把Channel注册到Poller（updateChannel）
// 3. 调用poll等待事件
// 4. poll返回后，处理活跃的Channel
class Poller : noncopyable {
public:
    // ChannelList就是Channel*的数组
    using ChannelList = std::vector<Channel*>;
    
    Poller();
    ~Poller();
    
    // 最核心的函数：等待事件发生
    // 参数：
    //   timeoutMs: 最多等待多少毫秒（-1表示永远等待）
    //   activeChannels: 输出参数，用来返回有事件的Channel
    void poll(int timeoutMs, ChannelList* activeChannels);
    
    // 更新Channel在epoll中的状态
    // 如果Channel是新的，就添加到epoll
    // 如果Channel已存在，就修改它关注的事件
    void updateChannel(Channel* channel);
    
    // 从epoll中移除Channel
    void removeChannel(Channel* channel);

private:
    int epollfd_;  // epoll的文件描述符（epoll_create返回的）
    
    // 这个map保存所有的Channel
    // key是fd，value是对应的Channel指针
    // 为什么需要这个map？因为epoll_wait返回的是fd，
    // 我们需要找到对应的Channel来处理事件
    using ChannelMap = std::map<int, Channel*>;
    ChannelMap channels_;
};

#endif