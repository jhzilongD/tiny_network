#ifndef TINY_NETWORK_LOGGER_LOGGER_H
#define TINY_NETWORK_LOGGER_LOGGER_H

#include "LogStream.h"
#include "../base/Timestamp.h"
#include <string>
#include <functional>

class AsyncLogging;

// Logger：日志系统的前端接口
//
// 用法：
//   LOG_INFO << "message " << var;
//   LOG_ERROR << "error occurred";
//
// 特点：
//   1. 自动添加时间戳、线程ID、文件位置
//   2. 支持流式输出（像std::cout）
//   3. 析构时自动输出到异步日志系统
class Logger {
public:
    // 日志级别
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    // 构造函数：记录日志元信息
    Logger(const char* file, int line, LogLevel level);
    Logger(const char* file, int line, LogLevel level, const char* func);
    ~Logger();

    // 获取格式化流
    LogStream& stream() { return stream_; }

    // 全局设置
    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);
    
    // 设置输出回调（默认输出到AsyncLogging）
    using OutputFunc = std::function<void(const char* msg, int len)>;
    static void setOutput(OutputFunc);
    
    // 设置刷新回调
    using FlushFunc = std::function<void()>;
    static void setFlush(FlushFunc);

private:
    // 初始化日志头部信息（时间戳、线程ID等）
    void formatTime();

    LogStream stream_;      // 格式化流
    LogLevel level_;        // 日志级别
    int line_;             // 行号
    const char* basename_;  // 文件名（去除路径）
};

// 全局日志级别
extern Logger::LogLevel g_logLevel;

// 判断日志级别是否应该输出
inline Logger::LogLevel Logger::logLevel() {
    return g_logLevel;
}

// 便利宏定义
#define LOG_TRACE if (Logger::logLevel() <= Logger::TRACE) \
  Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
  Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
  Logger(__FILE__, __LINE__, Logger::INFO).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()

#endif