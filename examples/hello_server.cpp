// 我们的第一个网络程序
// 让我们一步步来写

#include <iostream>
#include <sys/socket.h>  // 这个文件包含了socket函数
#include <netinet/in.h>  // 这个文件包含了地址结构体
#include <cstring>       // 这个文件包含了strlen函数
#include <unistd.h>      // 这个文件包含了close函数
#include <sys/epoll.h>   // 这个文件包含了epoll函数
#include <csignal>       // 这个文件包含了信号处理函数

// 全局变量，用来控制服务器是否继续运行
bool server_running = true;

// 信号处理函数（当按Ctrl+C时调用）
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n收到退出信号，正在优雅关闭服务器..." << std::endl;
        server_running = false;
    }
}

int main() {
    std::cout << "我要创建一个简单的服务器程序" << std::endl;
    
    // 注册信号处理函数（这样按Ctrl+C时会调用我们的函数）
    signal(SIGINT, signal_handler);
    std::cout << "按Ctrl+C可以优雅退出服务器" << std::endl;
    
    // 第1步：我们需要创建一个socket（就像买一部电话机）
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cout << "❌ 创建socket失败!" << std::endl;
        return 1;  // 程序退出，返回错误代码
    }
    std::cout << "✅ Socket创建成功!" << std::endl;
    
    // 设置socket选项，允许重用地址（避免"地址已在使用"错误）
    int reuse = 1;
    int setsockopt_result = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (setsockopt_result < 0) {
        std::cout << "❌ 设置socket选项失败!" << std::endl;
        return 1;
    }
    std::cout << "✅ Socket选项设置成功!" << std::endl;
    
    // 第2步：给socket绑定一个地址和端口号（就像给电话机分配号码）
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;        // 使用IPv4
    server_address.sin_addr.s_addr = INADDR_ANY; // 监听所有IP地址
    server_address.sin_port = htons(8080);       // 监听8080端口
    
    // 使用bind函数把socket和地址绑定
    int bind_result = bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (bind_result < 0) {
        std::cout << "❌ 绑定失败!" << std::endl;
        return 1;
    }
    std::cout << "✅ 绑定端口8080成功!" << std::endl;
    // 第3步：让socket开始监听（就像等待电话响）
    int listen_result = listen(server_socket, 5);
    if (listen_result < 0) {
        std::cout << "❌ 监听失败!" << std::endl;
        return 1;
    }
    std::cout << "✅ 服务器开始监听，等待连接..." << std::endl;
    
    // 创建epoll实例（雇佣管家）
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        std::cout << "❌ 创建epoll失败!" << std::endl;
        return 1;
    }
    std::cout << "✅ Epoll创建成功!" << std::endl;
    
    // 把服务器socket添加到epoll监控列表（告诉管家监控这个电话）
    struct epoll_event event;
    event.events = EPOLLIN;  // 监控读事件（对服务器socket来说就是有新连接）
    event.data.fd = server_socket;
    
    int add_result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event);
    if (add_result < 0) {
        std::cout << "❌ 添加服务器socket到epoll失败!" << std::endl;
        return 1;
    }
    std::cout << "✅ 服务器socket已添加到epoll监控!" << std::endl;
    
    std::cout << "可以用 telnet localhost 8080 测试" << std::endl;
    
    // 创建事件数组，用来接收epoll_wait的结果
    struct epoll_event events[10];  // 最多同时处理10个事件
    
    // 使用epoll循环处理多个客户端
    while (server_running) {
        std::cout << "\n等待事件发生..." << std::endl;
        
        // epoll_wait等待事件（管家报告有哪些电话响了）
        int event_count = epoll_wait(epoll_fd, events, 10, -1);
        if (event_count < 0) {
            std::cout << "❌ epoll_wait失败!" << std::endl;
            break;
        }
        
        std::cout << "✅ 有" << event_count << "个事件发生" << std::endl;
        
        // 处理每个事件
        for (int i = 0; i < event_count; i++) {
            int fd = events[i].data.fd;
            
            if (fd == server_socket) {
                // 服务器socket有事件 = 有新连接
                std::cout << "  -> 服务器socket有新连接!" << std::endl;
                
                struct sockaddr_in client_address;
                socklen_t client_address_length = sizeof(client_address);
                int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_length);
                
                if (client_socket >= 0) {
                    std::cout << "  -> ✅ 接受新连接，fd=" << client_socket << std::endl;
                    
                    const char* welcome_message = "欢迎连接到我的epoll服务器！请输入消息：\n";
                    send(client_socket, welcome_message, strlen(welcome_message), 0);
                    
                    // 把客户端socket加入epoll监控（监听客户端发送的数据）
                    struct epoll_event client_event;
                    client_event.events = EPOLLIN;  // 监控读事件
                    client_event.data.fd = client_socket;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &client_event);
                    
                    std::cout << "  -> 客户端socket已加入epoll监控，连接保持" << std::endl;
                }
            } else {
                // 客户端socket有事件 = 客户端发送了数据
                std::cout << "  -> 客户端fd=" << fd << "发送了数据" << std::endl;
                
                char buffer[1024];
                int bytes_received = recv(fd, buffer, sizeof(buffer)-1, 0);
                
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';  // 添加字符串结束符
                    std::cout << "  -> 收到消息: " << buffer << std::endl;
                    
                    // 回复客户端
                    const char* reply = "服务器收到了你的消息！\n";
                    send(fd, reply, strlen(reply), 0);
                } else {
                    // 客户端断开连接
                    std::cout << "  -> 客户端fd=" << fd << "断开连接" << std::endl;
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);  // 从epoll中移除
                    close(fd);
                }
            }
        }
    }
    
    // 清理资源
    close(epoll_fd);
    close(server_socket);
    std::cout << "服务器已优雅关闭" << std::endl;
    
    return 0;
}