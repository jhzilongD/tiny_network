#include "Acceptor.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"
#include "../logger/Logger.h"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

Acceptor::Acceptor(EventLoop* loop, int port)
    : loop_(loop),
      listening_(false)
{
    // 创建监听socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        LOG_ERROR << "Acceptor: socket() failed";
        return;
    }
    
    // 使用Socket封装
    acceptSocket_.reset(new Socket(listenfd));
    
    // 设置socket选项
    acceptSocket_->setReuseAddr(true);
    acceptSocket_->setReusePort(true);
    
    // 绑定地址
    InetAddress listenAddr(port);
    acceptSocket_->bindAddress(listenAddr);
    
    // 创建Channel管理listenfd
    channel_.reset(new Channel(listenfd));
    channel_->setReadCallback(
        std::bind(&Acceptor::handleRead, this));
    
    LOG_INFO << "Acceptor: listening on port " << port;
}

Acceptor::~Acceptor() {
    // Socket的析构函数会自动close
}

void Acceptor::listen() {
    // 开始监听
    acceptSocket_->listen();
    listening_ = true;
    
    // 注册到EventLoop，开始监听可读事件
    channel_->enableReading();
    loop_->updateChannel(channel_.get());
}

void Acceptor::handleRead() {
    // 有新连接到达
    InetAddress peerAddr(0);  // 临时构造，accept会重新设置
    int connfd = acceptSocket_->accept(&peerAddr);
    
    if (connfd >= 0) {
        LOG_INFO << "Acceptor: new connection from " << peerAddr.toIpPort() 
                 << ", fd=" << connfd;
        
        // 调用用户设置的回调
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd);
        } else {
            // 没有设置回调，关闭连接
            LOG_WARN << "Acceptor: no callback, closing connection";
            close(connfd);
        }
    } else {
        LOG_ERROR << "Acceptor: accept() failed";
    }
}