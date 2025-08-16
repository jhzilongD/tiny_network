#include "HttpRequest.h"
#include <cctype>  // for isspace

// 设置HTTP方法（从字符串解析）
bool HttpRequest::setMethod(const char* start, const char* end) {
    std::string m(start, end);  // 构造临时字符串
    if (m == "GET") {
        method_ = kGet;
    } else if (m == "POST") {
        method_ = kPost;
    } else if (m == "HEAD") {
        method_ = kHead;
    } else if (m == "PUT") {
        method_ = kPut;
    } else if (m == "DELETE") {
        method_ = kDelete;
    } else {
        method_ = kInvalid;
    }
    return method_ != kInvalid;  // 返回是否解析成功
}

// 获取方法字符串表示
const char* HttpRequest::methodString() const {
    switch(method_) {
        case kGet:    return "GET";
        case kPost:   return "POST";
        case kHead:   return "HEAD";
        case kPut:    return "PUT";
        case kDelete: return "DELETE";
        default:      return "UNKNOWN";
    }
}

// 添加请求头（复杂的字符串处理逻辑）
void HttpRequest::addHeader(const char* start, const char* colon, const char* end) {
    std::string field(start, colon);  // 头部名称
    ++colon;
    // 跳过冒号后的空格
    while (colon < end && isspace(*colon)) {
        ++colon;
    }
    std::string value(colon, end);    // 头部值
    // 去掉值末尾的空格
    while (!value.empty() && isspace(value[value.size()-1])) {
        value.resize(value.size()-1);
    }
    headers_[field] = value;
}

// 获取指定请求头的值
std::string HttpRequest::getHeader(const std::string& field) const {
    auto it = headers_.find(field);
    if (it != headers_.end()) {
        return it->second;
    }
    return "";  // 找不到返回空字符串
}