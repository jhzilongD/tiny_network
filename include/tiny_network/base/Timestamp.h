#ifndef TINY_NETWORK_BASE_TIMESTAMP_H
#define TINY_NETWORK_BASE_TIMESTAMP_H

#include <cstdint>
#include <string>

class Timestamp {
public:
    // 默认构造，时间戳为0
    Timestamp() : microSecondsSinceEpoch_(0) {}
    
    // 从微秒数构造
    explicit Timestamp(int64_t microSecondsSinceEpoch)
        : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}
    
    // 获取当前时间
    static Timestamp now();
    
    // 获取微秒数
    int64_t microSecondsSinceEpoch() const { 
        return microSecondsSinceEpoch_; 
    }
    
    // 转换为字符串
    std::string toString() const;
    
    // 常量：每秒的微秒数
    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t microSecondsSinceEpoch_;  // 自Unix纪元以来的微秒数
};

// 比较操作符（用于定时器）
inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline bool operator>(Timestamp lhs, Timestamp rhs) {
    return lhs.microSecondsSinceEpoch() > rhs.microSecondsSinceEpoch();
}

#endif