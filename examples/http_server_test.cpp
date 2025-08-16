#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "EventLoop.h"
#include <iostream>

// HTTP业务处理函数
void onRequest(const HttpRequest& req, HttpResponse* resp) {
    std::cout << "=== HTTP Request ===" << std::endl;
    std::cout << "Method: " << req.methodString() << std::endl;
    std::cout << "Path: " << req.path() << std::endl;
    std::cout << "Query: " << req.query() << std::endl;
    
    // 打印请求头
    const auto& headers = req.headers();
    for (const auto& header : headers) {
        std::cout << header.first << ": " << header.second << std::endl;
    }
    std::cout << std::endl;

    // 根据路径返回不同响应
    if (req.path() == "/") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "TinyNetwork/1.0");
        resp->setBody("<html><head><title>TinyNetwork HTTP Server</title></head>"
                     "<body><h1>Welcome to TinyNetwork!</h1>"
                     "<p>A simple HTTP server built with C++</p>"
                     "<p><a href='/hello'>Hello Page</a></p>"
                     "</body></html>");
    } 
    else if (req.path() == "/hello") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("text/plain");
        resp->addHeader("Server", "TinyNetwork/1.0");
        resp->setBody("Hello, World! This is TinyNetwork HTTP Server.\n");
    }
    else if (req.path() == "/api/json") {
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("application/json");
        resp->addHeader("Server", "TinyNetwork/1.0");
        resp->setBody(R"({"message":"Hello from TinyNetwork","version":"1.0","status":"success"})");
    }
    else {
        // 404 Not Found
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setContentType("text/html");
        resp->addHeader("Server", "TinyNetwork/1.0");
        resp->setBody("<html><body><h1>404 Not Found</h1><p>The requested page was not found.</p></body></html>");
        resp->setCloseConnection(true);
    }
}

int main() {
    std::cout << "Starting TinyNetwork HTTP Server..." << std::endl;
    
    // 创建事件循环
    EventLoop loop;
    
    // 创建HTTP服务器，监听8080端口
    HttpServer server(&loop, "TinyHttpServer", 8080);
    
    // 设置HTTP请求处理回调
    server.setHttpCallback(onRequest);
    
    // 启动服务器
    server.start();
    
    std::cout << "HTTP Server running on http://localhost:8080" << std::endl;
    std::cout << "Available endpoints:" << std::endl;
    std::cout << "  http://localhost:8080/        - Homepage" << std::endl;
    std::cout << "  http://localhost:8080/hello   - Hello page" << std::endl;
    std::cout << "  http://localhost:8080/api/json - JSON API" << std::endl;
    std::cout << std::endl;
    std::cout << "Press Ctrl+C to stop the server." << std::endl;
    
    // 开始事件循环
    loop.loop();
    
    return 0;
}