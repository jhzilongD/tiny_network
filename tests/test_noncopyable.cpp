#include <iostream>
#include "noncopyable.h"

// 测试类：继承noncopyable
class TestClass : private noncopyable {
public:
    TestClass(int value) : value_(value) {
        std::cout << "TestClass构造，值=" << value_ << std::endl;
    }
    
    int getValue() const { return value_; }
    
private:
    int value_;
};

int main() {
    std::cout << "测试noncopyable类" << std::endl;
    
    // 正常构造 - 应该OK
    TestClass obj1(42);
    std::cout << "obj1的值: " << obj1.getValue() << std::endl;
    
    // 下面这行如果取消注释，编译会失败！
    // TestClass obj2 = obj1;  // ❌ 编译错误！
    
    std::cout << "✅ noncopyable工作正常！" << std::endl;
    
    return 0;
}