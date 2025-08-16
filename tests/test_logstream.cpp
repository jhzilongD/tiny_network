#include "LogStream.h"
#include <iostream>

int main() {
    LogStream stream;
    
    // 测试各种类型
    stream << "Hello World! ";
    stream << "Number: " << 123 << ", ";
    stream << "Double: " << 3.14159 << ", ";
    stream << "Bool: " << true << ", ";
    stream << "Char: " << 'A';
    
    // 输出结果
    std::cout << "LogStream result: " << stream.buffer().toString() << std::endl;
    std::cout << "Length: " << stream.buffer().length() << std::endl;
    
    // 测试链式调用
    LogStream stream2;
    std::string name = "Tom";
    int age = 25;
    stream2 << "User " << name << " is " << age << " years old";
    
    std::cout << "Chain test: " << stream2.buffer().toString() << std::endl;
    
    return 0;
}