#ifndef TINY_NETWORK_BASE_THREAD_H
#define TINY_NETWORK_BASE_THREAD_H

#include <functional>
#include <string>
#include <thread>
#include <memory>
#include "noncopyable.h"

class Thread : private noncopyable {
public:
    using ThreadFunc = std::function<void()>;
    
    explicit Thread(ThreadFunc func, const std::string& name = std::string());
    ~Thread();
    
    // 启动线程
    void start();
    
    // 等待线程结束  
    void join();
    
    // 查询线程是否已启动
    bool started() const { return started_; }
    
    // 获取线程名
    const std::string& name() const { return name_; }

private:
    bool started_;                              // 线程是否已启动
    std::shared_ptr<std::thread> thread_;      // 实际的线程对象
    ThreadFunc func_;                          // 线程要执行的函数
    std::string name_;                         // 线程名
};

#endif