#include <iostream>
#include <chrono>
#include <sys/syscall.h>
#include <unistd.h>
#include "CurrentThread.h"

// 直接系统调用版本（用于对比）
int getDirectTid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void performanceTest() {
    const int iterations = 1000000;  // 100万次调用
    
    // 测试缓存版本性能
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        CurrentThread::tid();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto cached_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 测试直接系统调用版本性能
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        getDirectTid();
    }
    end = std::chrono::high_resolution_clock::now();
    auto direct_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "性能测试结果（" << iterations << "次调用）:" << std::endl;
    std::cout << "缓存版本耗时: " << cached_duration.count() << " 微秒" << std::endl;
    std::cout << "直接系统调用耗时: " << direct_duration.count() << " 微秒" << std::endl;
    std::cout << "性能提升: " << (double)direct_duration.count() / cached_duration.count() << "倍" << std::endl;
}

int main() {
    std::cout << "测试CurrentThread性能优化" << std::endl;
    
    // 验证功能正确性
    std::cout << "主线程TID (缓存版本): " << CurrentThread::tid() << std::endl;
    std::cout << "主线程TID (直接调用): " << getDirectTid() << std::endl;
    
    // 性能测试
    performanceTest();
    
    return 0;
}