#include "TcpServer.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "../logger/Logger.h"


// 构造函数：初始化服务器
TcpServer::TcpServer(EventLoop* loop,
                     const std::string& name,
                     int port)
    : loop_(loop),
      name_(name),
      port_(port),  // 保存端口号
      acceptor_(new Acceptor(loop, port)),  // 创建Acceptor
      threadPool_(new EventLoopThreadPool(loop, name + "-pool")),  // 创建线程池
      nextConnId_(1)  // 连接ID从1开始
{
    LOG_INFO << "TcpServer[" << name_ << "] created, port=" << port;
    
    // 设置Acceptor的新连接回调
    // 当有新连接时，Acceptor会调用newConnection
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer() {
    LOG_INFO << "TcpServer[" << name_ << "] destructing";
    
    // 清理所有连接
    for (auto& item : connections_) {
        auto conn = item.second;
        item.second.reset();  // 释放shared_ptr
        // TcpConnection的析构函数会关闭socket
    }
}

// 设置线程数量
void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}

// 启动服务器
void TcpServer::start() {
    LOG_INFO << "TcpServer[" << name_ << "] starting";
    
    // 启动线程池
    threadPool_->start();
    
    // 让Acceptor开始监听
    acceptor_->listen();
}

// 处理新连接（这是核心函数！）
void TcpServer::newConnection(int sockfd) {
    // 为新连接生成一个名字
    char buf[32];
    snprintf(buf, sizeof(buf), "-%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    
    LOG_INFO << "TcpServer::newConnection [" << connName << "] fd=" << sockfd;
    
    // 从线程池中选择一个EventLoop
    EventLoop* ioLoop = threadPool_->getNextLoop();
    LOG_DEBUG << "TcpServer: assign connection to EventLoop " << ioLoop;
    
    // 先在主线程创建TcpConnection对象
    // 注意：TcpConnection的构造函数只是初始化，不涉及IO操作
    auto conn = std::make_shared<TcpConnection>(ioLoop, connName, sockfd);
    
    // 在主线程保存到connections_中（线程安全）
    connections_[connName] = conn;
    
    // 设置各种回调（在主线程）- muduo风格
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    
    // 只让connectEstablished在IO线程执行
    ioLoop->runInLoop(
        std::bind(&TcpConnection::connectEstablished, conn));
}

// 移除连接（由TcpConnection在关闭时调用）
void TcpServer::removeConnection(const std::shared_ptr<TcpConnection>& conn) {
    // 在主线程执行移除操作
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

// 在主线程移除连接
void TcpServer::removeConnectionInLoop(const std::shared_ptr<TcpConnection>& conn) {
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_ << "] - connection " << conn->name();
    
    // 从主线程的map中删除
    connections_.erase(conn->name());
    
    // 在IO线程执行最后的清理
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
}

// 获取监听地址字符串
std::string TcpServer::ipPort() const {
    return "0.0.0.0:" + std::to_string(port_);
}