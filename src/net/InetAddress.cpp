#include "InetAddress.h"
#include <cstring>      // 为了使用 memset
#include <arpa/inet.h>  // 为了使用 inet_ntop

// 构造函数：根据端口号创建地址（监听所有接口）
InetAddress::InetAddress(uint16_t port) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = INADDR_ANY;
}

// 构造函数：指定IP和端口
InetAddress::InetAddress(const std::string& ip, uint16_t port) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
}

// 构造函数：从sockaddr_in构造
InetAddress::InetAddress(const struct sockaddr_in& addr)
    : addr_(addr) {
}

uint16_t InetAddress::toPort() const {
    return ntohs(addr_.sin_port);
}

std::string InetAddress::toIp() const {
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return std::string(buf);
}

std::string InetAddress::toIpPort() const {
    return toIp() + ":" + std::to_string(toPort());
}

const struct sockaddr* InetAddress::getSockAddr() const {
    return reinterpret_cast<const struct sockaddr*>(&addr_);
}

socklen_t InetAddress::getSockLen() const {
    return sizeof(addr_);
}