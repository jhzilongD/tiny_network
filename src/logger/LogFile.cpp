#include "LogFile.h"
#include "FileUtil.h"
#include "../base/Timestamp.h"
#include <unistd.h>
#include <mutex>
#include <assert.h>
#include <stdio.h>

LogFile::LogFile(const std::string& basename,
                 size_t rollSize,
                 bool threadSafe)
    : basename_(basename),
      rollSize_(rollSize),
      threadSafe_(threadSafe)
{
    if (threadSafe_) {
        mutex_.reset(new std::mutex);
    }
    
    // 立即打开第一个日志文件
    rollFile();
}

LogFile::~LogFile() {
    // FileUtil的析构函数会自动关闭文件
}

void LogFile::append(const char* logline, int len) {
    if (threadSafe_) {
        std::lock_guard<std::mutex> lock(*mutex_);
        append_unlocked(logline, len);
    } else {
        append_unlocked(logline, len);
    }
}

void LogFile::flush() {
    if (threadSafe_) {
        std::lock_guard<std::mutex> lock(*mutex_);
        if (file_) file_->flush();
    } else {
        if (file_) file_->flush();
    }
}

void LogFile::append_unlocked(const char* logline, int len) {
    assert(file_);
    
    // 使用FileUtil写入文件（内部用fwrite_unlocked优化）
    file_->write(logline, len);
    
    // 检查是否需要滚动文件
    if (file_->writtenBytes() >= rollSize_) {
        rollFile();
    }
}

void LogFile::rollFile() {
    // 生成新文件名
    std::string filename = getLogFileName(basename_);
    
    // 创建新的FileUtil（会自动关闭旧文件）
    file_.reset(new FileUtil(filename));
}

std::string LogFile::getLogFileName(const std::string& basename) {
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;
    
    // 添加时间戳
    char timebuf[32];
    Timestamp now = Timestamp::now();
    time_t seconds = static_cast<time_t>(now.microSecondsSinceEpoch() / 1000000);
    struct tm tm_time;
    localtime_r(&seconds, &tm_time);
    
    // 格式：basename.20231201-143022.log
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm_time);
    filename += timebuf;
    
    // 添加进程ID和扩展名
    char pidbuf[32];
    snprintf(pidbuf, sizeof pidbuf, "%d.log", getpid());
    filename += pidbuf;
    
    return filename;
}