// 测试TcpConnection
// 这个程序展示如何用TcpConnection管理连接

#include "EventLoop.h"
#include "Channel.h"
#include "TcpConnection.h"
#include "Buffer.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <memory>
#include <vector>

int main() {
    std::cout << "=== 测试TcpConnection ===" << std::endl;
    
    // 1. 创建监听socket（和之前一样）
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(7777);
    
    bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listenfd, 5);
    
    std::cout << "服务器监听端口7777..." << std::endl;
    std::cout << "可以用多个telnet连接测试" << std::endl;
    
    // 2. 创建EventLoop
    EventLoop loop;
    
    // 3. 保存所有的TcpConnection
    std::vector<std::unique_ptr<TcpConnection>> connections;
    int connId = 0;  // 连接计数器
    
    // 4. 监听socket的Channel
    Channel listenChannel(listenfd);
    
    listenChannel.setReadCallback([&]() {
        std::cout << "\n>>> 新连接到达！" << std::endl;
        
        // accept新连接
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &len);
        
        if (connfd >= 0) {
            // 为新连接创建TcpConnection
            std::string connName = "conn" + std::to_string(++connId);
            std::cout << "创建TcpConnection: " << connName << std::endl;
            
            auto conn = std::make_unique<TcpConnection>(&loop, connName, connfd);
            
            // 设置消息回调：收到消息时的处理
            conn->setMessageCallback([](const std::shared_ptr<TcpConnection>& conn, Buffer* buf) {
                // 从Buffer中读取数据
                std::string msg = buf->retrieveAsString();
                std::cout << "[" << conn->name() << "] 收到消息: " << msg;
                
                // 回显消息
                std::string echo = "服务器回显: " + msg;
                conn->send(echo);
                
                // 如果收到"quit"，退出（简化处理）
                if (msg.find("quit") != std::string::npos) {
                    std::cout << "收到quit，但简化版还不支持优雅退出" << std::endl;
                }
            });
            
            // 启动连接
            conn->connectEstablished();
            
            // 保存连接
            connections.push_back(std::move(conn));
            
            // 发送欢迎消息
            connections.back()->send("欢迎连接到TcpConnection测试服务器！\n");
        }
    });
    
    listenChannel.enableReading();
    loop.updateChannel(&listenChannel);
    
    // 5. 开始事件循环
    std::cout << "\n开始事件循环..." << std::endl;
    std::cout << "提示：这个简化版不会自动退出，需要Ctrl+C结束" << std::endl;
    
    loop.loop();
    
    // 6. 清理
    close(listenfd);
    
    return 0;
}