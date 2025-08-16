#ifndef TINY_NETWORK_NET_TCPCONNECTION_H
#define TINY_NETWORK_NET_TCPCONNECTION_H

#include "../base/noncopyable.h"
#include "Buffer.h"
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

class Channel;
class EventLoop;

// TcpConnection：代表一个已建立的TCP连接
// 
// 生命周期：
// 1. accept()后创建TcpConnection
// 2. 处理这个连接的所有读写事件
// 3. 连接断开时销毁TcpConnection
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    // === 连接状态枚举 ===
    enum StateE { 
        kDisconnected,    // 未连接
        kConnecting,      // 正在连接  
        kConnected,       // 已连接
        kDisconnecting    // 正在断开
    };

    // 回调函数类型定义
    using ConnectionCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>; // 连接建立/断开回调
    using MessageCallback = std::function<void(const std::shared_ptr<TcpConnection>&, Buffer*)>; // 消息回调
    using CloseCallback = std::function<void(const std::shared_ptr<TcpConnection>&)>; // 连接关闭回调（内部使用）
    
    // 构造函数
    // loop: 管理这个连接的EventLoop
    // name: 连接的名字（方便调试）
    // sockfd: accept()返回的已连接socket
    TcpConnection(EventLoop* loop,
                  const std::string& name,
                  int sockfd);
    
    ~TcpConnection();
    
    // === 基本信息获取 ===
    const std::string& name() const { return name_; }
    EventLoop* getLoop() const { return loop_; }
    
    // === 连接状态管理 ===
    bool connected() const { return state_ == kConnected; }
    StateE state() const { return state_; }
    
    // 优雅关闭连接（只关闭写端，允许读取剩余数据）
    void shutdown();
    
    // 强制关闭连接
    void forceClose();
    
    // === 数据发送接口 ===
    void send(const std::string& message);
    void send(Buffer* buf);  // 新增：支持Buffer发送
    
    // === 上下文存储接口 ===
    // 设置上下文（用于存储协议相关状态，如HttpContext）
    void setContext(const std::string& key, std::shared_ptr<void> context);
    
    // 获取上下文
    std::shared_ptr<void> getContext(const std::string& key);
    
    // 检查是否存在指定上下文
    bool hasContext(const std::string& key) const;
    
    // 清除指定上下文
    void clearContext(const std::string& key);
    
    // 设置连接回调（连接建立/断开时调用）
    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }
    
    // 设置消息回调（用户提供的业务逻辑）
    void setMessageCallback(const MessageCallback& cb) { 
        messageCallback_ = cb; 
    }
    
    // 设置关闭回调（TcpServer使用）
    void setCloseCallback(const CloseCallback& cb) {
        closeCallback_ = cb;
    }
    
    // 启动这个连接（开始监听事件）
    void connectEstablished();
    
    // 连接断开时调用（由TcpServer调用）
    void connectDestroyed();

private:
    // 处理读事件（Channel的回调）
    void handleRead();
    
    // 处理写事件（Channel的回调）
    void handleWrite();
    
    // 处理连接关闭
    void handleClose();
    
    EventLoop* loop_;              // 所属的EventLoop
    std::string name_;              // 连接名
    int sockfd_;                    // socket描述符
    std::unique_ptr<Channel> channel_;  // 管理sockfd的事件
    StateE state_;                  // 连接状态
    
    Buffer inputBuffer_;                 // 输入缓冲区（接收数据）
    Buffer outputBuffer_;                // 输出缓冲区（发送数据）
    
    ConnectionCallback connectionCallback_; // 连接建立/断开回调
    MessageCallback messageCallback_;       // 消息到达的回调
    CloseCallback closeCallback_;           // 连接关闭的回调
    
    // 上下文存储（key-value方式存储任意类型的上下文对象）
    std::unordered_map<std::string, std::shared_ptr<void>> contexts_;
};

#endif