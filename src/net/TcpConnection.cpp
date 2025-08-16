#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>

// 构造函数：初始化一个TCP连接
TcpConnection::TcpConnection(EventLoop* loop,
                           const std::string& name,
                           int sockfd)
    : loop_(loop),
      name_(name),
      sockfd_(sockfd),
      channel_(new Channel(sockfd)),  // 创建Channel管理这个sockfd
      state_(kConnecting)             // 初始状态为正在连接
{
    std::cout << "TcpConnection::ctor[" << name_ << "] fd=" << sockfd_ << std::endl;
    
    // 设置Channel的回调函数
    // 当sockfd可读时，Channel会调用handleRead
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this));
    // 当sockfd可写时，Channel会调用handleWrite
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this));
}

// 析构函数：清理资源
TcpConnection::~TcpConnection() {
    std::cout << "TcpConnection::dtor[" << name_ << "] fd=" << sockfd_ << std::endl;
    close(sockfd_);  // 关闭socket
}

// 处理读事件：读取数据并调用用户回调
void TcpConnection::handleRead() {
    // 使用Buffer读取数据
    ssize_t n = inputBuffer_.readFd(sockfd_);
    
    if (n > 0) {
        // 收到数据
        std::cout << "TcpConnection[" << name_ << "] recv " << n << " bytes" << std::endl;
        
        // 调用用户设置的消息回调
        // 用户负责从inputBuffer_中取出数据
        if (messageCallback_) {
            messageCallback_(shared_from_this(), &inputBuffer_);
        }
    } else if (n == 0) {
        // 对端关闭连接
        std::cout << "TcpConnection[" << name_ << "] peer closed" << std::endl;
        handleClose();  // 处理连接关闭
    } else {
        // 出错
        std::cout << "TcpConnection[" << name_ << "] recv error" << std::endl;
    }
}

// 发送数据
void TcpConnection::send(const std::string& message) {
    if (sockfd_ < 0) {
        std::cout << "TcpConnection[" << name_ << "] sockfd invalid" << std::endl;
        return;
    }
    
    size_t remaining = message.size();
    ssize_t nwrote = 0;
    
    // 如果输出缓冲区没有待发送数据，尝试直接发送
    if (outputBuffer_.readableBytes() == 0) {
        // 尝试直接发送
        nwrote = ::send(sockfd_, message.c_str(), message.size(), 0);
        
        if (nwrote >= 0) {
            remaining -= nwrote;
            std::cout << "TcpConnection[" << name_ << "] send " << nwrote << " bytes, "
                     << remaining << " bytes remaining" << std::endl;
            
            if (remaining == 0) {
                // 全部发送完成，完美！
                return;
            }
        } else {
            nwrote = 0;
            std::cout << "TcpConnection[" << name_ << "] send error" << std::endl;
            return;
        }
    }
    
    // 没有发送完，或者outputBuffer_本来就有数据
    // 把剩余数据（或全部数据）追加到缓冲区
    outputBuffer_.append(message.c_str() + nwrote, remaining);
    
    // 关注可写事件
    if (!channel_->isWriting()) {
        channel_->enableWriting();
        loop_->updateChannel(channel_.get());
        std::cout << "TcpConnection[" << name_ << "] enable writing" << std::endl;
    }
}

// 启动连接：注册到EventLoop开始监听事件
void TcpConnection::connectEstablished() {
    std::cout << "TcpConnection[" << name_ << "] connectEstablished" << std::endl;
    
    // 更新连接状态为已连接
    state_ = kConnected;
    
    // 让Channel开始监听可读事件
    channel_->enableReading();
    
    // 注册到EventLoop
    loop_->updateChannel(channel_.get());
    
    // 调用用户的连接回调（通知连接建立）
    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    }
}

// 处理写事件：发送缓冲区中的数据
void TcpConnection::handleWrite() {
    if (!channel_->isWriting()) {
        std::cout << "TcpConnection[" << name_ << "] handleWrite but not writing" << std::endl;
        return;
    }
    
    // 发送outputBuffer_中的数据
    ssize_t n = ::send(sockfd_, 
                      outputBuffer_.peek(), 
                      outputBuffer_.readableBytes(), 0);
    
    if (n > 0) {
        outputBuffer_.retrieve(n);
        std::cout << "TcpConnection[" << name_ << "] write " << n << " bytes, " 
                 << outputBuffer_.readableBytes() << " bytes remaining" << std::endl;
        
        if (outputBuffer_.readableBytes() == 0) {
            // 发送完成，停止关注可写事件
            channel_->disableWriting();
            loop_->updateChannel(channel_.get());
            std::cout << "TcpConnection[" << name_ << "] disable writing" << std::endl;
        }
    } else {
        std::cout << "TcpConnection[" << name_ << "] handleWrite error" << std::endl;
    }
}

// 处理连接关闭
void TcpConnection::handleClose() {
    std::cout << "TcpConnection[" << name_ << "] handleClose" << std::endl;
    
    // 停止监听所有事件
    channel_->disableAll();
    
    // 调用关闭回调（通知TcpServer移除这个连接）
    if (closeCallback_) {
        closeCallback_(shared_from_this());
    }
}

// 连接销毁（由TcpServer调用）
void TcpConnection::connectDestroyed() {
    std::cout << "TcpConnection[" << name_ << "] connectDestroyed" << std::endl;
    
    if (state_ == kConnected) {
        // 更新连接状态为已断开
        state_ = kDisconnected;
        
        // 通知用户连接断开了
        if (connectionCallback_) {
            connectionCallback_(shared_from_this());
        }
    }
    
    // 从EventLoop中移除Channel
    channel_->disableAll();
    // 注意：这里简化处理，实际应该从Poller中移除
}

// === 新增的连接控制方法 ===

// 优雅关闭连接（只关闭写端）
void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        state_ = kDisconnecting;
        // 关闭写端，允许继续读取
        ::shutdown(sockfd_, SHUT_WR);  
        std::cout << "TcpConnection[" << name_ << "] shutdown write end" << std::endl;
    }
}

// 强制关闭连接
void TcpConnection::forceClose() {
    if (state_ == kConnected || state_ == kDisconnecting) {
        state_ = kDisconnecting;
        // 直接触发关闭处理
        handleClose();
        std::cout << "TcpConnection[" << name_ << "] force close" << std::endl;
    }
}

// 发送Buffer数据
void TcpConnection::send(Buffer* buf) {
    if (state_ == kConnected) {
        // 直接从Buffer发送数据
        send(std::string(buf->peek(), buf->readableBytes()));
        buf->retrieveAll();  // 清空Buffer
    } else {
        std::cout << "TcpConnection[" << name_ << "] not connected, cannot send" << std::endl;
    }
}

// === 上下文存储功能实现 ===

// 设置上下文
void TcpConnection::setContext(const std::string& key, std::shared_ptr<void> context) {
    contexts_[key] = context;
    std::cout << "TcpConnection[" << name_ << "] setContext: " << key << std::endl;
}

// 获取上下文
std::shared_ptr<void> TcpConnection::getContext(const std::string& key) {
    auto it = contexts_.find(key);
    if (it != contexts_.end()) {
        return it->second;
    }
    return nullptr;  // 没找到返回空指针
}

// 检查是否存在指定上下文
bool TcpConnection::hasContext(const std::string& key) const {
    return contexts_.find(key) != contexts_.end();
}

// 清除指定上下文
void TcpConnection::clearContext(const std::string& key) {
    auto it = contexts_.find(key);
    if (it != contexts_.end()) {
        contexts_.erase(it);
        std::cout << "TcpConnection[" << name_ << "] clearContext: " << key << std::endl;
    }
}