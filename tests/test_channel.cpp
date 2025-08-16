#include <iostream>
#include <sys/epoll.h>
#include "Channel.h"

void readCallback() {
    std::cout << "读事件发生了！" << std::endl;
}

void writeCallback() {
    std::cout << "写事件发生了！" << std::endl;
}

void errorCallback() {
    std::cout << "错误事件发生了！" << std::endl;
}

int main() {
    std::cout << "测试Channel类" << std::endl;
    
    // 创建一个Channel（使用标准输入的fd=0）
    Channel channel(0);
    
    // 设置回调函数
    channel.setReadCallback(readCallback);
    channel.setWriteCallback(writeCallback);
    channel.setErrorCallback(errorCallback);
    
    // 启用读事件监听
    channel.enableReading();
    std::cout << "channel.events() = " << channel.events() << std::endl;
    std::cout << "预期值(EPOLLIN|EPOLLPRI) = " << (EPOLLIN | EPOLLPRI) << std::endl;
    
    // 模拟epoll返回读事件
    channel.set_revents(EPOLLIN);
    std::cout << "模拟读事件发生，调用handleEvent()" << std::endl;
    channel.handleEvent();
    
    // 模拟epoll返回错误事件
    channel.set_revents(EPOLLERR);
    std::cout << "模拟错误事件发生，调用handleEvent()" << std::endl;
    channel.handleEvent();
    
    return 0;
}