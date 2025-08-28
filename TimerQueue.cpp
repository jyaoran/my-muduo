/********************************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2025-08-27 22:23:02
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2025-08-28 22:15:01
 * @FilePath: /my-muduo/TimerQueue.cpp
 * @Description:
 * @
 * @Copyright (c) 2025 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved.
 *********************************************************************/
#include "TimerQueue.h"
#include "Timer.h"
#include "TimerId.h"
#include "EventLoop.h"
#include "easylogging++.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <assert.h>

namespace detail
{

    int createTimerfd()
    {
        int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                       TFD_NONBLOCK | TFD_CLOEXEC);
        if (timerfd < 0)
        {
            LOG(ERROR) << "Failed in timerfd_create";
        }
        return timerfd;
    }

    struct timespec howMuchTimeFromNow(Timestamp when)
    {
        int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
        if (microseconds < 100)
        {
            microseconds = 100;
        }
        struct timespec ts;
        ts.tv_sec = static_cast<time_t>(
            microseconds / Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(
            (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
        return ts;
    }

    void readTimerfd(int timerfd, Timestamp now)
    {
        uint64_t howmany;
        ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
        LOG(TRACE) << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
        if (n != sizeof howmany)
        {
            LOG(ERROR) << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
        }
    }

    void resetTimerfd(int timerfd, Timestamp expiration)
    {
        // wake up loop by timerfd_settime()
        struct itimerspec newValue;
        struct itimerspec oldValue;
        memset(&newValue, 0, sizeof(newValue));
        memset(&oldValue, 0, sizeof(oldValue));
        newValue.it_value = howMuchTimeFromNow(expiration);
        int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
        if (ret)
        {
            LOG(ERROR) << "timerfd_settime()";
        }
    }

} // namespace detail

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timerfd_(detail::createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      timers_(),
      callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));

    // 我们总是读取timerfd，通过timerfd_settime来 disarm（禁用）它。

    timerfdChannel_.enableReading(); // 通过Channel管理timerfd，并设置为可读
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll(); // 禁用timerfdChannel_的所有事件（读、写等）
    timerfdChannel_.remove();     // 从事件循环中移除timerfdChannel_
    ::close(timerfd_);            // 关闭timerfd

    // 不要移除channel，因为我们正在EventLoop的析构函数中
    // 遍历所有定时器并删除它们
    for (const Entry &timer : timers_)
    {
        delete timer.second; // 删除定时器对象，释放内存
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    // 创建一个新的Timer对象，移动回调函数以避免拷贝
    Timer *timer = new Timer(std::move(cb), when, interval);

    // 在事件循环中安全的添加定时器
    loop_->runInLoop(
        std::bind(&TimerQueue::addTimerInLoop, this, timer));

    // 返回TimerId对象
    return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId)
{
    // 确保在事件循环的线程中安全地取消定时器
    loop_->runInLoop(
        std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer *timer)
{
    // 确保当前线程是事件循环所在的线程
    loop_->assertInLoopThread();

    // 插入定时器，并返回时间顺序是否改变
    bool earliestChanged = insert(timer);

    // 如果新插入的定时器比现有的所有定时器都要早到期
    // 那么我们必须重置timerfd，让它在这个更早的时间触发
    if (earliestChanged)
    {
        detail::resetTimerfd(timerfd_, timers_.begin()->first);
    }
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    loop_->assertInLoopThread();

    // 创建一个临时变量，保存timer指针和序列号
    ActiveTimer timer(timerId.timer_, timerId.sequence_);

    // 找到timer指针在timers_中的位置
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if (it != activeTimers_.end())
    {
        // 找到了，从timers_中删除
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1); // 确保只移除了一个定时器
        (void)n;        // 抑制未使用变量的警告

        delete it->first; // 删除定时器对象，释放内存

        // 从activeTimers_中移除该定时器
        activeTimers_.erase(it);
    }
    else if (callingExpiredTimers_)
    {
        // 正在处理到期的定时器，放入cancelingTimers_
        // 以便在处理完到期定时器后取消它
        cancelingTimers_.insert(timer);
    }
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();

    // 获取当前时间
    Timestamp now(Timestamp::now());

    // 读取timerfd， 每次触发都必须进行读取，清楚epoll的通知状态
    detail::readTimerfd(timerfd_, now);

    // 获取所有到期的定时器
    std::vector<Entry> expired = getExpired(now);

    // 标记正在处理到期的定时器
    callingExpiredTimers_ = true;

    // 清空取消定时器集合
    cancelingTimers_.clear();

    // 处理回调，定时器到期需要做的事情
    for (const Entry &timer : expired)
    {
        timer.second->run(); // 执行定时器的回调函数
    }

    // 取消标记正在处理到期的定时器
    callingExpiredTimers_ = false;

    // 重置定时器队列
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;                                // 用于存储到期的定时器列表
    Entry sentry(now, reinterpret_cast<Timer *>(UINTPTR_MAX)); // 哨兵元素，用于结束循环

    // 使用lower_bound查找第一个到期时间不小于sentry的定时器
    TimerList::iterator end = timers_.lower_bound(sentry);

    // 将timers_中所有到期的定时器复制到expired向量中
    std::copy(timers_.begin(), end, back_inserter(expired));

    // 从timers_中移除已到期的定时器
    timers_.erase(timers_.begin(), end);

    // 遍历所有到期的定时器，并从activeTimers_中移除对应的定时器
    for (const Entry &it : expired)
    {
        // 创建一个ActiveTimer对象，使用到期的定时器及其序列号
        ActiveTimer timer(it.second, it.second->sequence());

        // 从activeTimers_中移除该定时器，并确保操作成功（即移除数量n为1）
        size_t n = activeTimers_.erase(timer);
        assert(n == 1);
        (void)n; // 确保移除成功，(void)n是为了抑制编译器的未使用变量警告
    }

    // 返回到期的定时器列表
    return expired;
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now)
{
    Timestamp nextExpire; // 下一个到期时间

    for (const Entry &it : expired)
    {
        // 创建一个定时器对象，用来查找是否为已经取消的定时器
        ActiveTimer timer(it.second, it.second->sequence());

        // 检擦定时器是否需要重复，并且是否已经取消
        if (it.second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end())
        {
            // 重启定时器并设置时间
            it.second->restart(now);

            // 将重启的定时器重新插入到timers_合集中
            insert(it.second);
        }
        else
        {
            // 如果定时器不是重复定时器或者在取消列表中，则删除定时器对象
            delete it.second; 
        }
        // 检查timers_是否为空，如果不为空，则获取下一个最早到期的定时器的到期时间
        if (!timers_.empty())
        {
            nextExpire = timers_.begin()->second->expiration();
        }

        // 如果下一个到期时间有效，则重置timerfd以在下一个到期时间触发
        if (nextExpire.valid())
        {
            detail::resetTimerfd(timerfd_, nextExpire);
        }
    }
}

bool TimerQueue::insert(Timer *timer)
{
    loop_->assertInLoopThread();

    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();

    // 检查需要插入的定时器是否比现有的所有定时器都要早到期
    if (it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }

    // 将定时器插入到timers_合集中
    {
        std::pair<TimerList::iterator, bool> result =
            timers_.insert(Entry(when, timer));
        assert(result.second); // 确保插入成功
        (void)result;          // 抑制未使用变量的警告
    }

    // 将定时器插入到activeTimers_集合中
    {
        std::pair<ActiveTimerSet::iterator, bool> result =
            activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
    }

    return earliestChanged;
}
