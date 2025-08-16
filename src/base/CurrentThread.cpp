#include "CurrentThread.h"
#include <stdio.h>

namespace CurrentThread {
    // 线程局部存储变量，初始值为0
    __thread int cachedTid = 0;
    __thread char tidString[32];
    __thread int tidStringLength = 0;
    
    void cacheTid() {
        if (cachedTid == 0) {
            // 使用系统调用获取内核线程ID
            cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
            // 同时格式化为字符串（用于日志）
            tidStringLength = snprintf(tidString, sizeof tidString, "%5d ", cachedTid);
        }
    }
}