#include <iostream>
#include "InetAddress.h"

int main() {
    std::cout << "测试InetAddress类" << std::endl;
    
    // 测试构造函数
    InetAddress addr(8080);
    
    std::cout << "✅ InetAddress构造成功!" << std::endl;
    
    // 测试获取端口号
    uint16_t port = addr.toPort();
    std::cout << "获取到的端口号: " << port << std::endl;
    
    // 测试获取IP地址
    std::string ip = addr.toIp();
    std::cout << "获取到的IP地址: " << ip << std::endl;
    
    return 0;
}