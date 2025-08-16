#include <iostream>
#include <unistd.h>  // 为了使用sleep
#include "Timestamp.h"

int main() {
    std::cout << "测试Timestamp类" << std::endl;
    
    // 测试1：获取当前时间
    Timestamp now = Timestamp::now();
    std::cout << "当前时间戳: " << now.toString() << std::endl;
    std::cout << "微秒数: " << now.microSecondsSinceEpoch() << std::endl;
    
    // 测试2：休眠1秒后再获取时间
    std::cout << "\n休眠1秒..." << std::endl;
    sleep(1);
    
    Timestamp later = Timestamp::now();
    std::cout << "1秒后时间戳: " << later.toString() << std::endl;
    
    // 测试3：比较操作符
    if (later > now) {
        std::cout << "✅ 时间比较正确：later > now" << std::endl;
    }
    
    // 测试4：计算时间差
    int64_t diff = later.microSecondsSinceEpoch() - now.microSecondsSinceEpoch();
    std::cout << "时间差: " << diff << " 微秒 (约" 
              << diff / 1000000.0 << "秒)" << std::endl;
    
    return 0;
}