// Echo服务器 - 展示TcpServer的简单用法
// 对比之前的test程序，看看TcpServer如何简化代码

#include "TcpServer.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "Buffer.h"
#include <iostream>

int main() {
    std::cout << "=== Echo Server using TcpServer ===" << std::endl;
    
    // 1. 创建EventLoop
    EventLoop loop;
    
    // 2. 创建TcpServer（多么简单！）
    TcpServer server(&loop, "EchoServer", 6666);
    
    // 3. 设置消息回调（业务逻辑）
    server.setMessageCallback(
        [](const std::shared_ptr<TcpConnection>& conn, Buffer* buf) {
            // 从Buffer中读取所有数据
            std::string msg = buf->retrieveAsString();
            std::cout << "[" << conn->name() << "] echo: " << msg;
            
            // 回显消息
            conn->send("Echo: " + msg);
            
            // 如果收到"quit"，关闭服务器
            if (msg == "quit\r\n" || msg == "quit\n") {
                std::cout << "Received quit command" << std::endl;
                // TODO: 优雅退出
            }
        });
    
    // 4. 启动服务器
    server.start();
    std::cout << "Server listening on port 6666" << std::endl;
    std::cout << "Use 'telnet localhost 6666' to test" << std::endl;
    
    // 5. 开始事件循环
    loop.loop();
    
    return 0;
}