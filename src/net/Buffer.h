#ifndef TINY_NETWORK_NET_BUFFER_H
#define TINY_NETWORK_NET_BUFFER_H

#include <vector>
#include <string>
#include <algorithm>

// Buffer：应用层缓冲区
// 
// 设计思路：
// |----已读数据----|----可读数据----|----可写空间----|
//                  ↑              ↑               ↑
//                readerIndex   writerIndex      size()
//
// 为什么需要Buffer？
// 1. TCP是流协议，没有消息边界
// 2. 一次read可能读到不完整的消息
// 3. 一次write可能无法发送所有数据
class Buffer {
public:
    static const size_t kInitialSize = 1024;
    
    Buffer()
        : buffer_(kInitialSize),
          readerIndex_(0),
          writerIndex_(0) {}
    
    // 可读字节数
    size_t readableBytes() const {
        return writerIndex_ - readerIndex_;
    }
    
    // 可写字节数
    size_t writableBytes() const {
        return buffer_.size() - writerIndex_;
    }
    
    // 返回可读数据的起始地址
    const char* peek() const {
        return &buffer_[readerIndex_];
    }
    
    // 读取len字节（移动读指针）
    void retrieve(size_t len) {
        if (len < readableBytes()) {
            readerIndex_ += len;
        } else {
            retrieveAll();
        }
    }
    
    // 读取所有数据
    void retrieveAll() {
        readerIndex_ = 0;
        writerIndex_ = 0;
    }
    
    // 读取数据并返回string
    std::string retrieveAsString() {
        std::string str(peek(), readableBytes());
        retrieveAll();
        return str;
    }
    
    // 添加数据到缓冲区
    void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }
    
    void append(const std::string& str) {
        append(str.data(), str.size());
    }
    
    // 从socket读取数据
    ssize_t readFd(int fd);
    
    // === HTTP解析专用方法 ===
    
    // 查找\r\n（HTTP行结束符）
    const char* findCRLF() const {
        const char* crlf = std::search(peek(), beginWrite(), "\r\n", "\r\n" + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }
    
    // 读取数据直到指定位置（不包括end）
    void retrieveUntil(const char* end) {
        retrieve(end - peek());
    }
    
private:
    char* beginWrite() {
        return &buffer_[writerIndex_];
    }
    
    const char* beginWrite() const {
        return &buffer_[writerIndex_];
    }
    
    // 确保有足够的可写空间
    void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
    }
    
    // 扩展缓冲区
    void makeSpace(size_t len) {
        if (writableBytes() + readerIndex_ < len) {
            // 需要扩展buffer
            buffer_.resize(writerIndex_ + len);
        } else {
            // 把已读数据清理掉，腾出空间
            size_t readable = readableBytes();
            std::copy(buffer_.begin() + readerIndex_,
                     buffer_.begin() + writerIndex_,
                     buffer_.begin());
            readerIndex_ = 0;
            writerIndex_ = readable;
        }
    }
    
    std::vector<char> buffer_;  // 存储数据
    size_t readerIndex_;         // 读位置
    size_t writerIndex_;         // 写位置
};

#endif