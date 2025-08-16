#include "TcpServer.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "Buffer.h"
#include "Logger.h"
#include "AsyncLogging.h"
#include <iostream>
#include <memory>

// 带日志的Echo服务器
class EchoServerWithLog {
public:
    EchoServerWithLog(int port) 
        : loop_(new EventLoop()),
          server_(loop_.get(), "EchoServer", port),
          asyncLog_("echo_server", 500*1000*1000)  // 500MB滚动
    {
        // 设置TcpServer的消息回调
        server_.setMessageCallback([this](std::shared_ptr<TcpConnection> conn, Buffer* buf) {
            onMessage(conn, buf);
        });
        
        // 设置线程数
        server_.setThreadNum(4);
        
        // 启动异步日志
        asyncLog_.start();
        
        // 设置日志输出到异步日志系统
        Logger::setOutput([this](const char* msg, int len) {
            asyncLog_.append(msg, len);
        });
        
        // 设置日志级别
        Logger::setLogLevel(Logger::DEBUG);
        
        LOG_INFO << "EchoServerWithLog created, port=" << port;
    }
    
    ~EchoServerWithLog() {
        LOG_INFO << "EchoServerWithLog shutting down";
        asyncLog_.stop();
    }
    
    void start() {
        LOG_INFO << "EchoServer starting...";
        server_.start();
        loop_->loop();  // 事件循环
    }
    
private:
    void onMessage(std::shared_ptr<TcpConnection> conn, Buffer* buf) {
        // 读取所有数据
        std::string message = buf->retrieveAsString();
        
        LOG_INFO << "Received from " << conn->name() << ": " << message.substr(0, message.size()-1);  // 去掉换行
        
        // Echo回去
        conn->send("Echo: " + message);
        
        // 如果收到"quit"，关闭连接
        if (message == "quit\n") {
            LOG_INFO << "Client " << conn->name() << " requested quit";
            // TcpConnection会在没有数据时自动关闭
            // 或者我们可以直接关闭socket
        }
    }

    std::unique_ptr<EventLoop> loop_;
    TcpServer server_;
    AsyncLogging asyncLog_;
};

int main() {
    std::cout << "=== Echo Server with Async Logging ===" << std::endl;
    std::cout << "Starting server on port 8080..." << std::endl;
    std::cout << "Test with: telnet localhost 8080" << std::endl;
    std::cout << "Logs will be written to echo_server.*.log files" << std::endl;
    
    try {
        EchoServerWithLog server(8080);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}