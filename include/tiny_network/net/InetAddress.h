#ifndef TINY_NETWORK_NET_INETADDRESS_H
#define TINY_NETWORK_NET_INETADDRESS_H

#include <netinet/in.h>  // 为了使用 sockaddr_in
#include <string>        // 为了使用 std::string

// 我们的第一个网络地址类
class InetAddress {
public:
    // 构造函数
    explicit InetAddress(uint16_t port);  // 监听所有接口
    InetAddress(const std::string& ip, uint16_t port);  // 指定IP和端口
    explicit InetAddress(const struct sockaddr_in& addr);  // 从sockaddr_in构造
    
    // 获取信息
    uint16_t toPort() const;
    std::string toIp() const;
    std::string toIpPort() const;  // IP:端口格式
    
    // 获取原始地址（给socket API使用）
    const struct sockaddr* getSockAddr() const;
    socklen_t getSockLen() const;

private:
    sockaddr_in addr_;  // 存储网络地址信息的核心！
};

#endif