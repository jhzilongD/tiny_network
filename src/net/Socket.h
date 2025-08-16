#ifndef TINY_NETWORK_NET_SOCKET_H
#define TINY_NETWORK_NET_SOCKET_H

#include "../base/noncopyable.h"

class InetAddress;

// Socket类：封装socket文件描述符
// 
// 职责：
// 1. RAII管理socket的生命周期
// 2. 封装常用的socket操作
// 3. 统一的错误处理
class Socket : noncopyable {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();
    
    int fd() const { return sockfd_; }
    
    // 服务端操作
    void bindAddress(const InetAddress& addr);
    void listen();
    int accept(InetAddress* peeraddr);
    
    // 通用操作
    void shutdownWrite();
    
    // 设置socket选项
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
    void setTcpNoDelay(bool on);  // 禁用Nagle算法

private:
    const int sockfd_;
};

#endif