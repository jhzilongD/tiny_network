#include "Thread.h"

Thread::Thread(ThreadFunc func, const std::string& name)
    : started_(false),
      thread_(nullptr),
      func_(std::move(func)),
      name_(name) {
}

Thread::~Thread() {
    if (started_ && thread_->joinable()) {
        thread_->detach();  // 如果线程还在运行，detach它
    }
}

void Thread::start() {
    started_ = true;
    thread_ = std::make_shared<std::thread>(func_);
}

void Thread::join() {
    if (started_ && thread_->joinable()) {
        thread_->join();
    }
}