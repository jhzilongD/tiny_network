#include "FileUtil.h"
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

FileUtil::FileUtil(const std::string& filename)
    : fp_(nullptr), writtenBytes_(0)
{
    // 以追加模式打开文件，设置close-on-exec标志
    fp_ = ::fopen(filename.c_str(), "ae");
    
    if (fp_) {
        // 设置64KB缓冲区（比系统默认4KB大）
        // 对于日志这种顺序写入场景，大缓冲区能显著减少系统调用
        ::setbuffer(fp_, buffer_, sizeof buffer_);
    } else {
        fprintf(stderr, "FileUtil: failed to open %s, errno=%d %s\n", 
                filename.c_str(), errno, strerror(errno));
    }
}

FileUtil::~FileUtil() {
    if (fp_) {
        ::fclose(fp_);
    }
}

size_t FileUtil::write(const char* data, size_t len) {
    assert(fp_);
    
    // 使用fwrite_unlocked替代fwrite
    // 在单线程环境下（异步日志的后台线程）性能更好
    // 因为省去了不必要的加锁开销
    size_t written = ::fwrite_unlocked(data, 1, len, fp_);
    
    if (written != len) {
        fprintf(stderr, "FileUtil: write error, expected=%zu, written=%zu, errno=%d %s\n",
                len, written, errno, strerror(errno));
    }
    
    writtenBytes_ += written;
    return written;
}

void FileUtil::flush() {
    assert(fp_);
    ::fflush(fp_);
}