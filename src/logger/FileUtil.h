#ifndef TINY_NETWORK_LOGGER_FILEUTIL_H
#define TINY_NETWORK_LOGGER_FILEUTIL_H

#include "../base/noncopyable.h"
#include <string>

// FileUtil：底层文件操作封装
// 
// 优化要点：
// 1. 使用fwrite_unlocked提升单线程性能
// 2. 64KB自定义缓冲区，替代系统默认4KB
// 3. 精确的写入字节数统计
// 4. 完善的错误处理
class FileUtil : noncopyable {
public:
    explicit FileUtil(const std::string& filename);
    ~FileUtil();

    // 追加数据到文件
    size_t write(const char* data, size_t len);
    
    // 刷新到磁盘
    void flush();
    
    // 获取已写入字节数
    off_t writtenBytes() const { return writtenBytes_; }

private:
    FILE* fp_;                    // 文件指针
    char buffer_[64 * 1024];      // 64KB缓冲区
    off_t writtenBytes_;          // 已写入字节数统计
};

#endif