#include "HttpResponse.h"
#include "../net/Buffer.h"  // 需要Buffer的完整定义
#include <stdio.h>          // for snprintf

void HttpResponse::appendToBuffer(Buffer* output) const {
    char buf[32];
    
    // 1. 构造状态行：HTTP/1.1 200 OK\r\n
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
    output->append(buf);
    output->append(statusMessage_);
    output->append("\r\n");
    
    // 2. 如果有响应体，自动添加Content-Length头
    if (!body_.empty()) {
        snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
        output->append(buf);
    }
    
    // 3. 添加所有响应头
    for (const auto& header : headers_) {
        output->append(header.first);        // 头部名称
        output->append(": ");
        output->append(header.second);       // 头部值
        output->append("\r\n");
    }
    
    // 4. 连接控制头
    if (closeConnection_) {
        output->append("Connection: close\r\n");
    } else {
        output->append("Connection: Keep-Alive\r\n");
    }
    
    // 5. 空行分隔头部和正文
    output->append("\r\n");
    
    // 6. 响应体
    output->append(body_);
}