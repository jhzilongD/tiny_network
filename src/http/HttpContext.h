#ifndef TINY_NETWORK_HTTP_HTTPCONTEXT_H
#define TINY_NETWORK_HTTP_HTTPCONTEXT_H

#include "HttpRequest.h"

class Buffer;  // 前向声明

class HttpContext {
public:
    // HTTP请求解析状态
    enum HttpRequestParseState {
        kExpectRequestLine,  // 等待解析请求行
        kExpectHeaders,      // 等待解析请求头部  
        kExpectBody,         // 等待解析请求体
        kGotAll             // 解析完成
    };

    HttpContext()
        : state_(kExpectRequestLine)
    {
    }

    // 核心解析函数：从Buffer中解析HTTP请求
    // 返回true表示解析成功，false表示需要更多数据
    bool parseRequest(Buffer* buf, Timestamp receiveTime);

    // 判断是否解析完成
    bool gotAll() const { 
        return state_ == kGotAll; 
    }

    // 重置解析状态（用于复用Context对象）
    void reset() {
        state_ = kExpectRequestLine;
        // 使用swap技术安全地清空request_
        HttpRequest dummy;
        request_.swap(dummy);
    }

    // 获取解析结果
    const HttpRequest& request() const { 
        return request_; 
    }
    
    HttpRequest& request() { 
        return request_; 
    }

private:
    // 解析请求行：GET /path?query HTTP/1.1
    bool processRequestLine(const char* begin, const char* end);

    HttpRequestParseState state_;  // 当前解析状态
    HttpRequest request_;          // 解析结果存储
};

#endif