#include "EventLoop.h"
#include "Thread.h"
#include "CurrentThread.h"
#include <iostream>
#include <unistd.h>

// 测试eventfd机制
// 这个测试会：
// 1. 在主线程创建EventLoop
// 2. 在另一个线程通过runInLoop投递任务
// 3. 验证任务在正确的线程执行

EventLoop* g_loop = nullptr;

void threadFunc() {
    std::cout << "\n[Thread " << CurrentThread::tid() 
              << "] 子线程启动，等待1秒..." << std::endl;
    sleep(1);
    
    std::cout << "[Thread " << CurrentThread::tid() 
              << "] 通过runInLoop向主线程投递任务..." << std::endl;
    
    // 向主线程的EventLoop投递任务
    g_loop->runInLoop([]() {
        std::cout << "[Thread " << CurrentThread::tid() 
                  << "] ✓ 任务在EventLoop线程中执行！" << std::endl;
        std::cout << "任务内容：Hello from another thread!" << std::endl;
    });
    
    std::cout << "[Thread " << CurrentThread::tid() 
              << "] 投递完成，再投递一个退出任务..." << std::endl;
    
    // 再投递一个任务：让EventLoop退出
    g_loop->runInLoop([]() {
        std::cout << "[Thread " << CurrentThread::tid() 
                  << "] 执行退出任务，EventLoop准备退出..." << std::endl;
        g_loop->quit();
    });
}

int main() {
    std::cout << "=== EventFD机制测试 ===" << std::endl;
    std::cout << "[Thread " << CurrentThread::tid() 
              << "] 主线程启动" << std::endl;
    
    // 创建EventLoop
    EventLoop loop;
    g_loop = &loop;
    
    std::cout << "[Thread " << CurrentThread::tid() 
              << "] EventLoop创建完成" << std::endl;
    
    // 创建子线程
    Thread thread(threadFunc, "TestThread");
    thread.start();
    
    std::cout << "[Thread " << CurrentThread::tid() 
              << "] 子线程已启动，开始事件循环..." << std::endl;
    std::cout << "EventLoop将会阻塞，直到收到eventfd通知\n" << std::endl;
    
    // 开始事件循环
    // 这里会阻塞，直到：
    // 1. 子线程通过eventfd唤醒它
    // 2. 执行子线程投递的任务
    // 3. 执行quit()退出循环
    loop.loop();
    
    std::cout << "\n[Thread " << CurrentThread::tid() 
              << "] EventLoop已退出" << std::endl;
    
    // 等待子线程结束
    thread.join();
    
    std::cout << "=== 测试完成 ===" << std::endl;
    std::cout << "\n测试结果解释：" << std::endl;
    std::cout << "1. 子线程通过runInLoop投递任务到主线程" << std::endl;
    std::cout << "2. eventfd机制唤醒了阻塞的EventLoop" << std::endl;
    std::cout << "3. 任务在正确的线程（主线程）中执行" << std::endl;
    std::cout << "4. 这就是eventfd实现跨线程通信的核心原理！" << std::endl;
    
    return 0;
}