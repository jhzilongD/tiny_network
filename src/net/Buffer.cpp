#include "Buffer.h"
#include <sys/uio.h>  // for readv
#include <unistd.h>
#include <errno.h>

// 从socket读取数据到Buffer
ssize_t Buffer::readFd(int fd) {
    // 使用栈上的临时缓冲区
    // 这样即使Buffer的空间不够，也能一次读取更多数据
    char extrabuf[65536];
    
    // 使用readv同时读到两个缓冲区
    struct iovec vec[2];
    const size_t writable = writableBytes();
    
    // 第一个缓冲区：Buffer的可写空间
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;
    
    // 第二个缓冲区：栈上的临时空间
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    
    // readv会按顺序填充缓冲区
    // 先填满vec[0]，再填vec[1]
    ssize_t n = ::readv(fd, vec, 2);
    
    if (n < 0) {
        // 读取出错
        return n;
    } else if (static_cast<size_t>(n) <= writable) {
        // Buffer的空间足够，数据都在vec[0]中
        writerIndex_ += n;
    } else {
        // Buffer空间不够，部分数据在extrabuf中
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    
    return n;
}