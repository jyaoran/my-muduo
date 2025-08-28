/********************************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2025-08-27 21:09:52
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2025-08-28 22:19:42
 * @FilePath: /my-muduo/TimerQueue.h
 * @Description:
 * @
 * @Copyright (c) 2025 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved.
 *********************************************************************/
#pragma once

#include <set>
#include <vector>

#include "Timer.h"
#include "Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"


class EventLoop;
class Timer;
class TimerId;

// 提供“尽力而为”的定时器队列。、
// 回调函数的执行时间不一定准确，因此不能保证每次执行都准时。
class TimerQueue : noncopyable
{
public:
    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    ///
    /// 调度回调函数在指定时间运行，
    /// 如果interval > 0.0，则定时器会重复触发。
    ///
    /// 必须是线程安全的，通常由其他线程调用。
    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);

    // 取消定时器
    // timerId 定时器ID
    void cancel(TimerId timerId);

private:
    using Entry = std::pair<Timestamp, Timer *>; // 定时器条目，包含到期时间和定时器指针
    using TimerList = std::set<Entry>;           // 定时器列表，按照到期时间排序
    using ActiveTimer = std::pair<Timer *, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    // 在事件循环中添加定时器
    void addTimerInLoop(Timer *timer); // 定时器指针

    // 在事件循环中取消定时器
    void cancelInLoop(TimerId timerId); // 定时器ID

    // 当timerfd触发时调用，处理定时器到期事件
    void handleRead();

    // 获取所有到期的定时器
    std::vector<Entry> getExpired(Timestamp now); // 当前时间

    // 重置定时器列表
    void reset(const std::vector<Entry> &expired, // 到期的定时器列表
               Timestamp now);                    // 当前时间

    // 插入定时器到timers_集合中
    bool insert(Timer *timer); // 定时器指针

    EventLoop *loop_;        // 指向事件循环的指针
    const int timerfd_;      // timerfd文件描述符
    Channel timerfdChannel_; // 管理timerfd的Channel对象
    
    TimerList timers_;       // 定时器列表，按到期时间排序

    // activeTimers_ ： 用于存储当前活跃的定时器，
    // 活跃的定时器是指那些已经被添加到定时器队列中，并且尚未到期或被取消的定时器。
    // 用途： 快速查找和管理活跃的定时器
    // 当定时器到期时，TimerQueue会从timers_集合中移除到期的定时器，并在activeTimers_集合中移除相应的条目
    // 在取消定时器时，TimerQueue会首先在activeTimers_集合中查找相应的条目，如果找到则进行取消操作
    ActiveTimerSet activeTimers_; // 活跃定时器集合

    std::atomic<bool> callingExpiredTimers_; // 原子标志，表示是否正在处理到期的定时器

    // cancelingTimers_ : 用于存储当前正在处理取消操作的定时器。
    // 这个合集主要用于在处理到期的定时器时，标记那些已经请求取消但尚未实际取消的定时器。
    // 用途 ： 当TimeQueue 正在处理到期定时器时，如果接收到取消请求，但该定时器尚未到期，
    // TimeQueue会将该定时器的标识符添加到cancelingTimers_集合中。
    // 在处理完到期定时器后，TimeQueue会检查cancelingTimers_集合，
    // 并取消其中的定时器。
    // 这种机制确保了在处理到期定时器的过程中，取消请求不会丢失
    ActiveTimerSet cancelingTimers_; // 正在取消的定时器集合
};