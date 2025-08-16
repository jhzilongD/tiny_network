#ifndef TINY_NETWORK_NET_TCPSERVER_H
#define TINY_NETWORK_NET_TCPSERVER_H

#include "../base/noncopyable.h"
#include <string>
#include <memory>
#include <map>
#include <functional>

class EventLoop;
class Acceptor;
class TcpConnection;
class Buffer;
class EventLoopThreadPool;

// TcpServer：用户使用的服务器类
// 
// 使用方式很简单：
// TcpServer server(&loop, "MyServer", 8080);
// server.setMessageCallback(处理消息);
// server.start();
class TcpServer : noncopyable {
public:
    // 类型定义
    using ConnectionPtr = std::shared_ptr<TcpConnection>;
    using MessageCallback = std::function<void(const ConnectionPtr&, Buffer*)>;
    using ConnectionCallback = std::function<void(const ConnectionPtr&)>;  // 连接建立/断开回调
    
    // 构造函数
    // loop: 事件循环
    // name: 服务器名称（用于日志）
    // port: 监听端口
    TcpServer(EventLoop* loop, 
             const std::string& name,
             int port);
    
    ~TcpServer();
    
    // === 基本信息获取 ===
    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    std::string ipPort() const;  // 获取监听地址字符串
    
    // === 回调设置 ===
    // 设置消息回调（用户的业务逻辑）
    void setMessageCallback(const MessageCallback& cb) { 
        messageCallback_ = cb; 
    }
    
    // 设置连接回调（连接建立/断开时调用）
    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }
    
    // === 服务器控制 ===
    // 设置IO线程数量（0表示所有IO都在主线程）
    void setThreadNum(int numThreads);
    
    // 启动服务器
    void start();

private:
    // 处理新连接（Acceptor会调用这个）
    void newConnection(int sockfd);
    
    // 移除连接（连接断开时调用）
    void removeConnection(const ConnectionPtr& conn);
    void removeConnectionInLoop(const ConnectionPtr& conn);
    
    EventLoop* loop_;                      // 主事件循环（Acceptor所在）
    const std::string name_;               // 服务器名称
    const int port_;                       // 监听端口
    std::unique_ptr<Acceptor> acceptor_;   // 负责accept
    std::unique_ptr<EventLoopThreadPool> threadPool_;  // IO线程池
    
    MessageCallback messageCallback_;      // 用户的消息处理函数
    ConnectionCallback connectionCallback_; // 用户的连接处理函数
    
    // 保存所有的连接
    // key是连接名，value是TcpConnection
    std::map<std::string, ConnectionPtr> connections_;
    int nextConnId_;  // 连接计数器
};

#endif