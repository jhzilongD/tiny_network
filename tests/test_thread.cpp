#include <iostream>
#include <unistd.h>
#include "Thread.h"

void threadFunction1() {
    std::cout << "线程1开始执行" << std::endl;
    sleep(2);
    std::cout << "线程1结束" << std::endl;
}

void threadFunction2() {
    std::cout << "线程2开始执行" << std::endl;
    sleep(1);
    std::cout << "线程2结束" << std::endl;
}

int main() {
    std::cout << "测试Thread类" << std::endl;
    
    // 创建两个线程
    Thread t1(threadFunction1, "worker1");
    Thread t2(threadFunction2, "worker2");
    
    std::cout << "线程名: " << t1.name() << ", 已启动: " << t1.started() << std::endl;
    
    // 启动线程
    t1.start();
    t2.start();
    
    std::cout << "线程已启动: " << t1.started() << std::endl;
    
    // 等待线程结束
    t1.join();
    t2.join();
    
    std::cout << "✅ 所有线程执行完毕" << std::endl;
    
    return 0;
}