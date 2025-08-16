#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>  // for strerror

// 创建eventfd，用于唤醒EventLoop
static int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        std::cerr << "Failed to create eventfd: " << strerror(errno) << std::endl;
        abort();
    }
    return evtfd;
}

// 构造函数
EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(new Poller()),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(wakeupFd_))  // Channel只需要fd
{
    std::cout << "EventLoop created in thread " << threadId_ << std::endl;
    
    // 设置wakeupChannel的读事件回调
    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handleRead, this));
    // 注册wakeupChannel到poller，监听读事件
    wakeupChannel_->enableReading();
}

// 析构函数
EventLoop::~EventLoop() {
    std::cout << "EventLoop destroyed" << std::endl;
    
    // 关闭wakeupChannel
    wakeupChannel_->disableAll();
    // Channel不需要remove，由析构函数自动处理
    
    // 关闭eventfd
    ::close(wakeupFd_);
}

// 核心函数：事件循环
void EventLoop::loop() {
    std::cout << "EventLoop::loop() started" << std::endl;
    
    // 确保不会重复进入loop
    if (looping_) {
        std::cerr << "EventLoop::loop() already looping!" << std::endl;
        return;
    }
    
    looping_ = true;
    quit_ = false;
    
    // 这就是Reactor的主循环！
    while (!quit_) {
        // 清空activeChannels，准备接收新的活跃Channel
        activeChannels_.clear();
        
        // 调用poll等待事件
        // 10秒超时，避免永久阻塞
        poller_->poll(10000, &activeChannels_);
        
        // 处理所有活跃的Channel
        for (Channel* channel : activeChannels_) {
            channel->handleEvent();
        }
        
        // 执行待处理的回调函数
        // 这些是其他线程通过runInLoop/queueInLoop添加的
        doPendingFunctors();
    }
    
    looping_ = false;
    std::cout << "EventLoop::loop() stopped" << std::endl;
}

// 退出事件循环
void EventLoop::quit() {
    quit_ = true;
    
    // 如果不是在EventLoop线程中调用quit，需要唤醒
    if (!isInLoopThread()) {
        wakeup();
    }
}

// 在EventLoop线程中执行回调
void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        // 如果在当前EventLoop线程，直接执行
        cb();
    } else {
        // 否则放入队列，唤醒EventLoop线程执行
        queueInLoop(std::move(cb));
    }
}

// 把回调放入队列
void EventLoop::queueInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    
    // 如果不在EventLoop线程，或者正在执行待处理回调，唤醒
    // 为什么callingPendingFunctors_时也要唤醒？
    // 因为如果正在执行pending functors，此时又添加了新的回调，
    // 需要再次唤醒以确保新回调及时执行
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

// 唤醒EventLoop
void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        std::cerr << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << std::endl;
    }
}

// 处理eventfd的读事件
void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        std::cerr << "EventLoop::handleRead() reads " << n << " bytes instead of 8" << std::endl;
    }
}

// 执行待处理的回调函数
void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    
    // 交换，减少锁的持有时间
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    
    // 执行回调
    for (const Functor& functor : functors) {
        functor();
    }
    
    callingPendingFunctors_ = false;
}

// 更新Channel（转发给Poller）
void EventLoop::updateChannel(Channel* channel) {
    poller_->updateChannel(channel);
}

// 移除Channel
void EventLoop::removeChannel(Channel* channel) {
    poller_->removeChannel(channel);
}