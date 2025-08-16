#include "Logger.h"
#include "AsyncLogging.h"
#include "../base/CurrentThread.h"
#include <iostream>
#include <string.h>

// 全局变量
Logger::LogLevel g_logLevel = Logger::INFO;

const char* LogLevelName[Logger::NUM_LOG_LEVELS] = {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

// 默认输出函数（输出到stdout）
void defaultOutput(const char* msg, int len) {
    fwrite(msg, 1, len, stdout);
}

// 默认刷新函数
void defaultFlush() {
    fflush(stdout);
}

// 全局输出和刷新函数
Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

// 提取文件名（去除路径）
const char* getBaseName(const char* file) {
    const char* slash = strrchr(file, '/');
    if (slash) {
        return slash + 1;
    }
    return file;
}

Logger::Logger(const char* file, int line, LogLevel level)
    : stream_(),
      level_(level),
      line_(line),
      basename_(getBaseName(file))
{
    formatTime();           // 时间戳
    CurrentThread::tid();   // 确保线程ID已缓存
    stream_ << CurrentThread::getTidString();  // 线程ID
    stream_ << LogLevelName[level];         // 日志级别
}

Logger::Logger(const char* file, int line, LogLevel level, const char* func)
    : stream_(),
      level_(level),
      line_(line),
      basename_(getBaseName(file))
{
    formatTime();
    CurrentThread::tid();   // 确保线程ID已缓存
    stream_ << CurrentThread::getTidString();
    stream_ << LogLevelName[level];
    stream_ << func << ' ';  // 函数名
}

Logger::~Logger() {
    // 添加文件名和行号
    stream_ << " - " << basename_ << ':' << line_ << '\n';
    
    // 获取完整日志内容
    const LogStream::Buffer& buf = stream_.buffer();
    
    // 输出日志
    g_output(buf.data(), buf.length());
    
    // FATAL级别需要abort
    if (level_ == FATAL) {
        g_flush();
        abort();
    }
}

void Logger::formatTime() {
    Timestamp now = Timestamp::now();
    time_t seconds = static_cast<time_t>(now.microSecondsSinceEpoch() / 1000000);
    int microseconds = static_cast<int>(now.microSecondsSinceEpoch() % 1000000);
    
    struct tm tm_time;
    localtime_r(&seconds, &tm_time);
    
    char buf[64];
    snprintf(buf, sizeof buf, "%4d%02d%02d %02d:%02d:%02d.%06d ",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
    
    stream_ << buf;
}

void Logger::setLogLevel(Logger::LogLevel level) {
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out) {
    g_output = out;
}

void Logger::setFlush(FlushFunc flush) {
    g_flush = flush;
}