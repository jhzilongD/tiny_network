#ifndef TINY_NETWORK_BASE_CURRENTTHREAD_H
#define TINY_NETWORK_BASE_CURRENTTHREAD_H

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread {
    // 线程局部存储，每个线程有自己的副本
    extern __thread int cachedTid;
    extern __thread char tidString[32];
    extern __thread int tidStringLength;
    
    // 缓存当前线程ID
    void cacheTid();
    
    // 获取当前线程ID（高效版本）
    inline int tid() {
        // 编译器优化提示：条件很少为真
        if (__builtin_expect(cachedTid == 0, 0)) {
            cacheTid();
        }
        return cachedTid;
    }
    
    // 获取线程ID字符串（用于日志）
    inline const char* getTidString() {
        return tidString;
    }
    
    inline int getTidStringLength() {
        return tidStringLength;
    }
}

#endif