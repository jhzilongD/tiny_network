#ifndef TINY_NETWORK_HTTP_HTTPRESPONSE_H
#define TINY_NETWORK_HTTP_HTTPRESPONSE_H

#include <string>
#include <unordered_map>

class Buffer;  // 前向声明，避免包含Buffer.h

class HttpResponse {
public:
    // HTTP状态码枚举（常用的几个）
    enum HttpStatusCode {
        kUnknown,
        k200Ok = 200,                    // 成功
        k301MovedPermanently = 301,      // 永久重定向
        k400BadRequest = 400,            // 客户端请求错误
        k404NotFound = 404,              // 资源不存在
        k500InternalServerError = 500    // 服务器内部错误
    };

    // 构造函数
    explicit HttpResponse(bool close)
        : statusCode_(kUnknown),
          closeConnection_(close)
    {
    }

    // === 设置响应信息（用户业务逻辑调用） ===
    
    void setStatusCode(HttpStatusCode code) {
        statusCode_ = code;
    }
    
    void setStatusMessage(const std::string& message) {
        statusMessage_ = message;
    }
    
    void setCloseConnection(bool on) {
        closeConnection_ = on;
    }
    
    // 便利方法：设置Content-Type
    void setContentType(const std::string& contentType) {
        addHeader("Content-Type", contentType);
    }
    
    // 添加响应头
    void addHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }
    
    // 设置响应体
    void setBody(const std::string& body) {
        body_ = body;
    }

    // === 获取响应信息 ===
    
    bool closeConnection() const {
        return closeConnection_;
    }
    
    HttpStatusCode statusCode() const {
        return statusCode_;
    }

    // === 核心功能：生成HTTP响应文本 ===
    
    // 将响应转换为HTTP格式并写入Buffer
    void appendToBuffer(Buffer* output) const;

private:
    std::unordered_map<std::string, std::string> headers_;  // 响应头
    HttpStatusCode statusCode_;                             // 状态码
    std::string statusMessage_;                             // 状态描述
    bool closeConnection_;                                  // 是否关闭连接
    std::string body_;                                      // 响应体
};

#endif