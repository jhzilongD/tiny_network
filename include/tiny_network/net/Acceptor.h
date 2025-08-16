#ifndef TINY_NETWORK_NET_ACCEPTOR_H
#define TINY_NETWORK_NET_ACCEPTOR_H

#include "../base/noncopyable.h"
#include <functional>
#include <memory>

class EventLoop;
class Channel;
class Socket;
class InetAddress;

// Acceptor：专门负责接受新连接
// 封装了listen socket的所有操作
class Acceptor : noncopyable {
public:
    // 新连接到达时的回调
    // 参数是accept返回的connfd
    using NewConnectionCallback = std::function<void(int sockfd)>;
    
    Acceptor(EventLoop* loop, int port);
    ~Acceptor();
    
    // 设置新连接回调
    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }
    
    // 开始监听
    void listen();
    
    bool listening() const { return listening_; }

private:
    // 处理新连接（Channel的读事件回调）
    void handleRead();
    
    EventLoop* loop_;
    std::unique_ptr<Socket> acceptSocket_;  // 监听socket的封装
    std::unique_ptr<Channel> channel_;      // 监听socket的Channel
    NewConnectionCallback newConnectionCallback_;  // 用户回调
    bool listening_;
};

#endif