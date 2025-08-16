#include "Logger.h"
#include "AsyncLogging.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "=== 测试同步日志（输出到终端） ===" << std::endl;
    
    // 1. 测试基本日志输出
    LOG_INFO << "Hello Logger!";
    LOG_ERROR << "This is an error message";
    LOG_WARN << "Warning: " << 42;
    
    // 2. 测试不同级别
    Logger::setLogLevel(Logger::DEBUG);
    LOG_DEBUG << "Debug message";
    LOG_TRACE << "Trace message";
    
    std::cout << "\n=== 测试异步日志（输出到文件） ===" << std::endl;
    
    // 3. 启动异步日志
    AsyncLogging asyncLog("test_async", 500*1000*1000);  // 500MB滚动
    
    // 设置输出到异步日志
    Logger::setOutput([&](const char* msg, int len) {
        asyncLog.append(msg, len);
    });
    
    asyncLog.start();
    
    // 4. 测试异步日志写入
    LOG_INFO << "Async logging started";
    
    // 5. 多线程测试
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([i]() {
            for (int j = 0; j < 1000; ++j) {
                LOG_INFO << "Thread " << i << " message " << j;
            }
        });
    }
    
    // 等待线程完成
    for (auto& t : threads) {
        t.join();
    }
    
    LOG_INFO << "All threads finished";
    
    // 6. 停止异步日志
    std::this_thread::sleep_for(std::chrono::seconds(2));  // 让后台线程处理完
    asyncLog.stop();
    
    std::cout << "测试完成！检查当前目录下的 test_async.*.log 文件" << std::endl;
    
    return 0;
}