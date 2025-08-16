#ifndef TINY_NETWORK_HTTP_HTTPSERVER_H
#define TINY_NETWORK_HTTP_HTTPSERVER_H

#include "../base/noncopyable.h"
#include "../base/Timestamp.h"
#include "../net/TcpServer.h"
#include <functional>
#include <string>

class HttpRequest;
class HttpResponse;
class Buffer;
class TcpConnection;

class HttpServer : noncopyable {
public:
    // HTTP业务回调函数类型
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    // 构造函数（适配TcpServer接口）
    HttpServer(EventLoop* loop, 
              const std::string& name,
              int port);
    
    ~HttpServer();

    // 获取事件循环
    EventLoop* getLoop() const { 
        return server_.getLoop(); 
    }

    // 设置HTTP业务回调（用户提供）
    void setHttpCallback(const HttpCallback& cb) {
        httpCallback_ = cb;
    }
    
    // 启动服务器
    void start();

private:
    // TcpServer的回调函数（内部使用）
    void onConnection(const std::shared_ptr<TcpConnection>& conn);
    void onMessage(const std::shared_ptr<TcpConnection>& conn,
                   Buffer* buf,
                   Timestamp receiveTime);
    
    // 处理完整的HTTP请求
    void onRequest(const std::shared_ptr<TcpConnection>& conn, 
                   const HttpRequest& req);

    TcpServer server_;              // 底层TCP服务器
    HttpCallback httpCallback_;     // 用户的HTTP业务回调
};

#endif