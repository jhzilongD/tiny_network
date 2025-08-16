#ifndef TINY_NETWORK_HTTP_HTTPREQUEST_H
#define TINY_NETWORK_HTTP_HTTPREQUEST_H

#include "../base/Timestamp.h"
#include <string>
#include <unordered_map>

class HttpRequest {
public:
    // HTTP方法枚举 - 用枚举比字符串更高效和类型安全
    enum Method { 
        kInvalid,   // 无效方法
        kGet,       // GET请求
        kPost,      // POST请求
        kHead,      // HEAD请求
        kPut,       // PUT请求
        kDelete     // DELETE请求
    };
    
    // HTTP版本枚举
    enum Version { 
        kUnknown,   // 未知版本
        kHttp10,    // HTTP/1.0
        kHttp11     // HTTP/1.1
    };

    // 构造函数 - 初始化为无效状态
    HttpRequest()
        : method_(kInvalid),
          version_(kUnknown)
    {        
    }

    // === 设置方法（解析器会用到） ===
    
    // 设置HTTP版本
    void setVersion(Version v) {
        version_ = v;
    }
    
    // 设置HTTP方法（从字符串解析）
    bool setMethod(const char* start, const char* end);
    
    // 设置请求路径
    void setPath(const char* start, const char* end) {
        path_.assign(start, end);
    }
    
    // 设置查询参数 (?name=value&key=data)
    void setQuery(const char* start, const char* end) {
        query_.assign(start, end);
    }
    
    // 设置接收时间
    void setReceiveTime(Timestamp t) {
        receiveTime_ = t;
    }
    
    // 添加请求头
    void addHeader(const char* start, const char* colon, const char* end);

    // === 获取方法（用户业务逻辑会用到） ===
    
    Method method() const { return method_; }
    Version version() const { return version_; }
    const std::string& path() const { return path_; }
    const std::string& query() const { return query_; }
    Timestamp receiveTime() const { return receiveTime_; }
    
    // 获取方法字符串表示
    const char* methodString() const;
    
    // 获取指定请求头的值
    std::string getHeader(const std::string& field) const;
    
    // 获取所有请求头
    const std::unordered_map<std::string, std::string>& headers() const {
        return headers_;
    }
    
    // 交换两个HttpRequest对象（异常安全）
    void swap(HttpRequest& rhs) {
        std::swap(method_, rhs.method_);
        std::swap(version_, rhs.version_);
        path_.swap(rhs.path_);
        query_.swap(rhs.query_);
        std::swap(receiveTime_, rhs.receiveTime_);
        headers_.swap(rhs.headers_);
    }

private:
    Method method_;                    // HTTP方法
    Version version_;                  // HTTP版本
    std::string path_;                 // 请求路径 /api/hello
    std::string query_;                // 查询参数 name=world&key=value
    Timestamp receiveTime_;            // 请求接收时间
    std::unordered_map<std::string, std::string> headers_;  // 请求头键值对
};

#endif