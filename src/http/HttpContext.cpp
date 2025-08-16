#include "HttpContext.h"
#include "../net/Buffer.h"
#include <algorithm>  // for std::find

// 解析请求行：GET /path?query HTTP/1.1
bool HttpContext::processRequestLine(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    
    // 1. 解析HTTP方法（GET, POST等）
    const char* space = std::find(start, end, ' ');
    if (space != end && request_.setMethod(start, space)) {
        start = space + 1;  // 跳过空格
        
        // 2. 解析URL路径和参数
        space = std::find(start, end, ' ');
        if (space != end) {
            // 查找查询参数分隔符'?'
            const char* question = std::find(start, space, '?');
            if (question != space) {
                // 有查询参数：/path?name=value
                request_.setPath(start, question);
                request_.setQuery(question + 1, space);  // 跳过'?'
            } else {
                // 无查询参数：/path
                request_.setPath(start, space);
            }
            
            // 3. 解析HTTP版本
            start = space + 1;
            succeed = (end - start == 8 && std::equal(start, end - 1, "HTTP/1."));
            if (succeed) {
                if (*(end - 1) == '1') {
                    request_.setVersion(HttpRequest::kHttp11);
                } else if (*(end - 1) == '0') {
                    request_.setVersion(HttpRequest::kHttp10);
                } else {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}

// 核心解析函数：状态机驱动
bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime) {
    bool ok = true;
    bool hasMore = true;
    
    // 状态机循环，每次处理一行或一个状态
    while (hasMore) {
        if (state_ == kExpectRequestLine) {
            // 状态1：解析请求行
            const char* crlf = buf->findCRLF();
            if (crlf) {
                // 找到完整的请求行
                ok = processRequestLine(buf->peek(), crlf);
                if (ok) {
                    request_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf + 2);  // 消费请求行（包括\r\n）
                    state_ = kExpectHeaders;       // 转到下一状态
                } else {
                    hasMore = false;  // 解析失败，退出循环
                }
            } else {
                hasMore = false;  // 请求行不完整，等待更多数据
            }
        } else if (state_ == kExpectHeaders) {
            // 状态2：解析请求头部
            const char* crlf = buf->findCRLF();
            if (crlf) {
                const char* colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    // 找到头部行：Key: Value
                    request_.addHeader(buf->peek(), colon, crlf);
                } else {
                    // 空行表示头部结束
                    state_ = kGotAll;  // 简化：暂时不处理请求体
                    hasMore = false;
                }
                buf->retrieveUntil(crlf + 2);  // 消费这一行
            } else {
                hasMore = false;  // 头部行不完整，等待更多数据
            }
        } else if (state_ == kExpectBody) {
            // TODO: 处理请求体（暂时跳过）
            // 实际项目中需要根据Content-Length解析请求体
            state_ = kGotAll;
            hasMore = false;
        }
    }
    
    return ok;
}