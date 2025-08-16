#include "Socket.h"
#include "InetAddress.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <iostream>

Socket::~Socket() {
    close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr) {
    int ret = bind(sockfd_, addr.getSockAddr(), addr.getSockLen());
    if (ret < 0) {
        std::cerr << "Socket::bindAddress failed" << std::endl;
    }
}

void Socket::listen() {
    int ret = ::listen(sockfd_, 128);  // 全连接队列最大128
    if (ret < 0) {
        std::cerr << "Socket::listen failed" << std::endl;
    }
}

int Socket::accept(InetAddress* peeraddr) {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    
    // 从全连接队列取出一个连接
    int connfd = ::accept(sockfd_, 
                         reinterpret_cast<struct sockaddr*>(&addr), 
                         &len);
    
    if (connfd >= 0) {
        // 设置对端地址
        *peeraddr = InetAddress(addr);
    }
    
    return connfd;
}

void Socket::shutdownWrite() {
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        std::cerr << "Socket::shutdownWrite failed" << std::endl;
    }
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
                &optval, sizeof(optval));
}

void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                &optval, sizeof(optval));
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                &optval, sizeof(optval));
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                &optval, sizeof(optval));
}