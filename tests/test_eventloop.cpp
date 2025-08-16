// 测试EventLoop
// 对比test_poller_channel.cpp，看看EventLoop如何简化代码

#include "EventLoop.h"
#include "Channel.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>

int main() {
    std::cout << "=== 测试EventLoop ===" << std::endl;
    
    // 1. 创建服务器socket（和之前一样）
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8888);
    
    bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listenfd, 5);
    
    std::cout << "服务器监听端口8888..." << std::endl;
    
    // 2. 创建EventLoop（代替了Poller）
    EventLoop loop;
    
    // 3. 创建Channel并设置回调（和之前一样）
    Channel listenChannel(listenfd);
    
    listenChannel.setReadCallback([&listenfd, &loop]() {
        std::cout << "\n>>> 新连接到达！" << std::endl;
        
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int connfd = accept(listenfd, (struct sockaddr*)&client_addr, &len);
        
        if (connfd >= 0) {
            std::cout << "    接受连接，fd=" << connfd << std::endl;
            
            const char* msg = "欢迎！输入'quit'退出服务器\n";
            send(connfd, msg, strlen(msg), 0);
            
            // 读取客户端消息
            char buf[128] = {0};
            int n = recv(connfd, buf, sizeof(buf)-1, 0);
            if (n > 0) {
                std::cout << "    收到消息: " << buf;
                
                // 如果收到quit，退出EventLoop
                if (strncmp(buf, "quit", 4) == 0) {
                    std::cout << "    收到退出命令！" << std::endl;
                    loop.quit();
                }
            }
            
            close(connfd);
        }
    });
    
    listenChannel.enableReading();
    
    // 4. 注册到EventLoop（而不是Poller）
    loop.updateChannel(&listenChannel);
    
    // 5. 启动一个定时器线程，10秒后自动退出
    std::thread timer([&loop]() {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "\n定时器：10秒到，退出EventLoop" << std::endl;
        loop.quit();
    });
    timer.detach();
    
    // 6. 开始事件循环（这一行代替了之前的while循环！）
    std::cout << "\n开始事件循环（10秒后自动退出）..." << std::endl;
    loop.loop();  // 会阻塞在这里，直到quit()被调用
    
    // 7. 清理
    std::cout << "事件循环结束，清理资源..." << std::endl;
    close(listenfd);
    
    return 0;
}