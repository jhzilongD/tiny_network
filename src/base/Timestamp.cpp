#include "Timestamp.h"
#include <sys/time.h>  // 为了使用gettimeofday
#include <cstdio>      // 为了使用snprintf

// 获取当前时间
Timestamp Timestamp::now() {
    struct timeval tv;
    // gettimeofday获取当前时间，精度到微秒
    gettimeofday(&tv, nullptr);
    
    // tv.tv_sec是秒，tv.tv_usec是微秒
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

// 转换为字符串格式："秒.微秒"
std::string Timestamp::toString() const {
    char buf[32] = {0};
    int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
    
    // 格式化为 "秒.微秒"，微秒部分固定6位，不足补0
    snprintf(buf, sizeof(buf), "%ld.%06ld", seconds, microseconds);
    return buf;
}