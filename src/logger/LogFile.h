#ifndef TINY_NETWORK_LOGGER_LOGFILE_H
#define TINY_NETWORK_LOGGER_LOGFILE_H

#include "../base/noncopyable.h"
#include <string>
#include <memory>
#include <mutex>

class FileUtil;

// LogFile：负责日志文件的实际写入
// 
// 职责：
// 1. 管理日志文件的打开/关闭
// 2. 提供高效的文件写入接口
// 3. 支持日志文件滚动（按大小）
// 4. 优化文件I/O性能
class LogFile : noncopyable {
public:
    // basename: 日志文件基础名（不含路径和扩展名）
    // rollSize: 文件滚动大小（字节），默认500MB
    // threadSafe: 是否线程安全（异步日志设为false）
    LogFile(const std::string& basename,
            size_t rollSize = 500*1000*1000,
            bool threadSafe = true);
    
    ~LogFile();

    // 追加日志数据
    void append(const char* logline, int len);
    
    // 强制刷新到磁盘
    void flush();
    
    // 滚动到新文件
    void rollFile();

private:
    // 生成日志文件名：basename.20231201-143022.123.log
    static std::string getLogFileName(const std::string& basename);
    
    // 实际的追加实现
    void append_unlocked(const char* logline, int len);

    const std::string basename_;    // 文件名前缀
    const size_t rollSize_;         // 滚动大小
    const bool threadSafe_;         // 是否线程安全
    
    std::unique_ptr<std::mutex> mutex_;   // 线程安全锁
    std::unique_ptr<FileUtil> file_;      // 文件操作封装
};

#endif