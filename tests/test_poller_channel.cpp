// 测试Channel和Poller的配合
// 这个程序创建一个简单的TCP服务器，使用Channel和Poller处理连接

#include "Channel.h"
#include "Poller.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

int main() {
    std::cout << "=== 测试Channel + Poller ===" << std::endl;
    
    // 1. 创建服务器socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        std::cerr << "socket() failed" << std::endl;
        return -1;
    }
    
    // 设置地址重用
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    // 2. 绑定地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9999);
    
    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "bind() failed" << std::endl;
        close(listenfd);
        return -1;
    }
    
    // 3. 开始监听
    if (listen(listenfd, 5) < 0) {
        std::cerr << "listen() failed" << std::endl;
        close(listenfd);
        return -1;
    }
    
    std::cout << "服务器监听端口9999..." << std::endl;
    std::cout << "使用telnet localhost 9999测试" << std::endl;
    
    // 4. 创建Poller
    Poller poller;
    
    // 5. 创建监听socket的Channel
    Channel listenChannel(listenfd);
    
    // 设置可读回调（有新连接时触发）
    listenChannel.setReadCallback([&listenfd]() {
        std::cout << "\n>>> 监听Channel可读事件：有新连接！" << std::endl;
        
        // accept新连接
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &len);
        
        if (connfd >= 0) {
            std::cout << "    接受新连接，fd=" << connfd << std::endl;
            
            // 发送欢迎消息
            const char* msg = "欢迎！这是Channel+Poller测试服务器\n";
            send(connfd, msg, strlen(msg), 0);
            
            // 简单起见，这里直接关闭连接
            // 实际应该创建新的Channel来处理这个连接
            close(connfd);
            std::cout << "    为简化测试，已关闭连接" << std::endl;
        }
    });
    
    // 设置错误回调
    listenChannel.setErrorCallback([]() {
        std::cout << ">>> 监听Channel错误事件" << std::endl;
    });
    
    // 6. 让Channel关注可读事件
    listenChannel.enableReading();
    
    // 7. 注册到Poller
    poller.updateChannel(&listenChannel);
    
    // 8. 事件循环
    std::cout << "\n开始事件循环..." << std::endl;
    
    std::vector<Channel*> activeChannels;
    bool running = true;
    int loop_count = 0;
    
    while (running) {
        // 等待事件（最多等5秒）
        std::cout << "\n[Loop " << ++loop_count << "] 等待事件..." << std::endl;
        poller.poll(5000, &activeChannels);
        
        // 处理所有活跃的Channel
        for (Channel* channel : activeChannels) {
            std::cout << "处理活跃Channel，fd=" << channel->fd() << std::endl;
            channel->handleEvent();
        }
        
        // 运行10次循环后退出（50秒）
        if (loop_count >= 10) {
            std::cout << "\n测试完成，退出..." << std::endl;
            running = false;
        }
    }
    
    // 9. 清理
    close(listenfd);
    
    return 0;
}