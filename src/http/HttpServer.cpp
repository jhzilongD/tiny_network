#include "HttpServer.h"
#include "HttpContext.h"
#include "HttpRequest.h" 
#include "HttpResponse.h"
#include "../net/TcpConnection.h"
#include "../net/Buffer.h"
#include <iostream>

// HttpContext在连接对象中的存储key
const std::string kHttpContext = "HttpContext";

HttpServer::HttpServer(EventLoop* loop,
                       const std::string& name,
                       int port)
    : server_(loop, name, port)
{
    // 设置TcpServer的回调函数
    server_.setConnectionCallback([this](const std::shared_ptr<TcpConnection>& conn) {
        onConnection(conn);
    });
    
    server_.setMessageCallback([this](const std::shared_ptr<TcpConnection>& conn, 
                                     Buffer* buf) {
        onMessage(conn, buf, Timestamp::now());
    });
}

HttpServer::~HttpServer() {
}

void HttpServer::start() {
    std::cout << "HttpServer[" << server_.name() << "] starts listening on " 
              << server_.ipPort() << std::endl;
    server_.start();
}

// 连接建立/断开时的处理
void HttpServer::onConnection(const std::shared_ptr<TcpConnection>& conn) {
    if (conn->state() == TcpConnection::kConnecting || conn->state() == TcpConnection::kConnected) {
        // 新连接建立：为每个连接创建一个HttpContext
        std::shared_ptr<HttpContext> context = std::make_shared<HttpContext>();
        conn->setContext(kHttpContext, context);
        
        std::cout << "New HTTP connection: " << conn->name() << std::endl;
    } else {
        // 连接断开：HttpContext会自动销毁（智能指针）
        std::cout << "HTTP connection closed: " << conn->name() << std::endl;
    }
}

// 收到数据时的处理（核心逻辑）
void HttpServer::onMessage(const std::shared_ptr<TcpConnection>& conn,
                          Buffer* buf,
                          Timestamp receiveTime) {
    // 1. 获取这个连接的HttpContext
    auto context = std::static_pointer_cast<HttpContext>(
        conn->getContext(kHttpContext));
    
    if (!context) {
        std::cout << "Error: HttpContext not found for connection " 
                  << conn->name() << std::endl;
        conn->shutdown();
        return;
    }
    
    // 2. 使用HttpContext解析HTTP请求
    if (!context->parseRequest(buf, receiveTime)) {
        // 解析失败，发送400 Bad Request
        std::cout << "HTTP parse error from " << conn->name() << std::endl;
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
        return;
    }
    
    // 3. 检查是否解析完成
    if (context->gotAll()) {
        // 解析完成，处理HTTP请求
        onRequest(conn, context->request());
        
        // 重置Context，为下一个请求做准备（HTTP/1.1 keep-alive）
        context->reset();
    }
    // 如果没解析完成，继续等待更多数据
}

// 处理完整的HTTP请求
void HttpServer::onRequest(const std::shared_ptr<TcpConnection>& conn,
                          const HttpRequest& req) {
    const std::string& connection = req.getHeader("Connection");
    // HTTP/1.1默认keep-alive，HTTP/1.0默认close
    bool close = (connection == "close" || 
                  (req.version() == HttpRequest::kHttp10 && connection != "Keep-Alive"));
    
    // 创建HTTP响应对象
    HttpResponse response(close);
    
    // 调用用户提供的业务回调
    if (httpCallback_) {
        httpCallback_(req, &response);
    } else {
        // 没有设置回调，返回404
        response.setStatusCode(HttpResponse::k404NotFound);
        response.setStatusMessage("Not Found");
        response.setCloseConnection(true);
    }
    
    // 将HTTP响应转换为文本并发送
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    
    // 根据HTTP协议决定是否关闭连接
    if (response.closeConnection()) {
        conn->shutdown();
    }
}