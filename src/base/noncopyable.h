#ifndef TINY_NETWORK_BASE_NONCOPYABLE_H
#define TINY_NETWORK_BASE_NONCOPYABLE_H

// 禁止拷贝的基类
// 任何继承这个类的类都无法被拷贝
class noncopyable {
public:
    // 删除拷贝构造函数
    noncopyable(const noncopyable&) = delete;
    
    // 删除赋值操作符
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    // 默认构造和析构函数设为protected
    // 这样只有子类能调用，外部不能直接创建noncopyable对象
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif