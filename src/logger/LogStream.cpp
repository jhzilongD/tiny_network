#include "LogStream.h"
#include <algorithm>
#include <limits>
#include <type_traits>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

// 高效数字转字符串算法（参考muduo优化）
namespace detail {

// 数字字符查表，支持负数索引
const char digits[] = {'9', '8', '7', '6', '5', '4', '3', '2', '1', '0',
                       '1', '2', '3', '4', '5', '6', '7', '8', '9'};
const char* zero = digits + 9;  // 指向'0'，zero[-1]='9', zero[1]='1'

// 高性能整数转字符串
template<typename T>
size_t convert(char buf[], T value) {
    T i = value;
    char* p = buf;

    // 使用查表法处理每一位数字
    do {
        int lsd = static_cast<int>(i % 10);  // 最低位数字
        i /= 10;
        *p++ = zero[lsd];  // 直接查表，无需条件判断
    } while (i != 0);

    // 处理负数
    if (value < 0) {
        *p++ = '-';
    }
    
    *p = '\0';
    std::reverse(buf, p);  // 原地反转

    return p - buf;
}

} // namespace detail

// 整数格式化模板实现
template<typename T>
void LogStream::formatInteger(T v) {
    if (buffer_.avail() >= kMaxNumericSize) {
        size_t len = detail::convert(buffer_.current(), v);
        buffer_.add(len);
    }
}

// bool类型
LogStream& LogStream::operator<<(bool v) {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
}

// 各种整数类型
LogStream& LogStream::operator<<(short v) {
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v) {
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v) {
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v) {
    formatInteger(v);
    return *this;
}

// 浮点数类型（使用snprintf，精度足够）
LogStream& LogStream::operator<<(float v) {
    *this << static_cast<double>(v);
    return *this;
}

LogStream& LogStream::operator<<(double v) {
    if (buffer_.avail() >= kMaxNumericSize) {
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
        buffer_.add(len);
    }
    return *this;
}

// 字符类型
LogStream& LogStream::operator<<(char v) {
    buffer_.append(&v, 1);
    return *this;
}

// 字符串类型
LogStream& LogStream::operator<<(const char* str) {
    if (str) {
        buffer_.append(str, strlen(str));
    } else {
        buffer_.append("(null)", 6);
    }
    return *this;
}

LogStream& LogStream::operator<<(const unsigned char* str) {
    return operator<<(reinterpret_cast<const char*>(str));
}

LogStream& LogStream::operator<<(const std::string& v) {
    buffer_.append(v.c_str(), v.size());
    return *this;
}