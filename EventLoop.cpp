/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-18 14:15:40
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-29 10:42:27
 * @FilePath: /studyMuduo/EventLoop.cpp
 * @Description: 
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#include "EventLoop.h"
#include "Poller.h"
#include "easylogging++/easylogging++.h"
#include "TimerQueue.h"

#include <sys/eventfd.h>
#include <errno.h>

__thread EventLoop *t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000;

int createEventFd()
{
    int evtfd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (evtfd < 0)
    {
        // LOG(ERROR) << "eventfd error : " << errno;
    }
    return evtfd;
}


EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctor_(false),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventFd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentActiveChannel_(nullptr)
{
    // LOG(DEBUG) << "EventLoop create " << this << "in thread " << threadId_;
    if (!t_loopInThisThread)
    {
        t_loopInThisThread = this;
    }
    
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    while (!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for (Channel *channel : activeChannels_)
        {
            // poller监听那些channel发生了事件， 然后上报给EvnetLoop，通知channel处理相应的事件
            channel->handleEvent(pollReturnTime_);
        }

        // 执行当前EventLoop事件循环需要处理的回调操作
        doPendingFunctors();
    }
    
}

void EventLoop::quit()
{
    quit_ = true;

    // 如果是在其它线程中调通quit方法，例如在subLoop(worker)中，调用了mainLoop中的quit。
    // 就需要调用wakeup唤醒poll的监听，然后退出。
    if (!isInLoopThread())
    {
        wakeup();
    }
    
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
        // LOG(DEBUG) << "EventLoop::runInLoop tid = " << threadId_;
    }
    
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctor_.emplace_back(cb);
    }

    if (!isInLoopThread() || callingPendingFunctor_)
    {
        wakeup();
    }
    
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        // LOG(ERROR) << "writes " << n << " bytes instead of 8";
    }
    
}

void EventLoop::updateChannel(Channel *channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        // LOG(ERROR) << "reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctor_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctor_);
    }

    for (const Functor &functor : functors)
    {
        functor();
    }

    callingPendingFunctor_ = false;
}

void EventLoop::abortNotInLoopThread()
{
  LOG(FATAL) << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  CurrentThread::tid();
}


TimerId EventLoop::runAt(Timestamp time, TimerCallback cb)
{
    // 在指定时间运行定时器事件
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}
TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
    // 计算定时器到期的时间
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
    // 计算定时器到期的时间
    Timestamp time(addTime(Timestamp::now(), interval));

    return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId)
{
    // 取消指定的定时器
    return timerQueue_->cancel(timerId);
}