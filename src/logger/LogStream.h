#ifndef TINY_NETWORK_LOGGER_LOGSTREAM_H
#define TINY_NETWORK_LOGGER_LOGSTREAM_H

#include "../base/noncopyable.h"
#include <string>
#include <string.h>

// LogStream：高性能的日志格式化流
// 类似std::cout，但专为日志优化：
// 1. 固定4K栈缓冲区，无动态分配
// 2. 支持operator<<链式调用
// 3. 优化的数字转字符串
class LogStream : noncopyable {
public:
    // 固定大小缓冲区模板
    template<int SIZE>
    class FixedBuffer : noncopyable {
    public:
        FixedBuffer() : cur_(data_) {}

        // 添加数据到缓冲区
        void append(const char* buf, size_t len) {
            if (avail() > len) {
                memcpy(cur_, buf, len);
                cur_ += len;
            }
        }

        // 获取缓冲区数据
        const char* data() const { return data_; }
        int length() const { return static_cast<int>(cur_ - data_); }

        // 缓冲区操作
        char* current() { return cur_; }
        int avail() const { return static_cast<int>(end() - cur_); }
        void add(size_t len) { cur_ += len; }

        // 重置
        void reset() { cur_ = data_; }
        void bzero() { memset(data_, 0, sizeof data_); }

        // 转换为string（调试用）
        std::string toString() const { return std::string(data_, length()); }

    private:
        const char* end() const { return data_ + sizeof data_; }

        char data_[SIZE];  // 固定大小数组
        char* cur_;        // 当前写入位置
    };

public:
    // 4K缓冲区，足够一条日志
    using Buffer = FixedBuffer<4000>;

    LogStream() : buffer_() {}

    // operator<<重载 - 支持各种类型
    LogStream& operator<<(bool v);
    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);
    LogStream& operator<<(float v);
    LogStream& operator<<(double v);
    LogStream& operator<<(char c);
    LogStream& operator<<(const char* str);
    LogStream& operator<<(const unsigned char* str);
    LogStream& operator<<(const std::string& str);

    // 直接添加数据
    void append(const char* data, int len) { buffer_.append(data, len); }

    // 获取缓冲区
    const Buffer& buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

private:
    // 整数转字符串的模板函数
    template<typename T>
    void formatInteger(T);

    Buffer buffer_;
    static const int kMaxNumericSize = 32;  // 数字转字符串最大长度
};

#endif