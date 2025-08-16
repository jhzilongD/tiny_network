// 多线程Echo服务器 - 展示多线程TcpServer
#include "TcpServer.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "Buffer.h"
#include <iostream>
#include <thread>

int main(int argc, char* argv[]) {
    std::cout << "=== Multi-threaded Echo Server ===" << std::endl;
    
    // 获取线程数量（默认4个IO线程）
    int numThreads = 4;
    if (argc > 1) {
        numThreads = atoi(argv[1]);
    }
    
    // 1. 创建EventLoop（主线程）
    EventLoop loop;
    
    // 2. 创建TcpServer
    TcpServer server(&loop, "MTEchoServer", 6666);
    
    // 3. 设置IO线程数量
    server.setThreadNum(numThreads);
    std::cout << "IO thread number: " << numThreads << std::endl;
    
    // 4. 设置消息回调
    server.setMessageCallback(
        [](const std::shared_ptr<TcpConnection>& conn, Buffer* buf) {
            // 从Buffer中读取所有数据
            std::string msg = buf->retrieveAsString();
            
            // 显示当前线程ID，证明多线程工作
            std::cout << "[Thread " << std::this_thread::get_id() 
                     << "] [" << conn->name() << "] echo: " << msg;
            
            // 回显消息
            conn->send("Echo: " + msg);
        });
    
    // 5. 启动服务器
    server.start();
    std::cout << "Server listening on port 6666" << std::endl;
    std::cout << "Main thread: " << std::this_thread::get_id() << std::endl;
    
    // 6. 开始事件循环
    loop.loop();
    
    return 0;
}