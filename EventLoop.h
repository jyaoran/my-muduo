/***********************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-18 14:15:40
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-19 15:10:51
 * @FilePath: /studyMuduo/EventLoop.h
 * @Description:
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved.
 ************************************************************/
#pragma once

#include <atomic>
#include <functional>
#include <vector>
#include <memory>
#include <mutex>

#include "CurrentThread.h"
#include "Timestamp.h"
#include "noncopyable.h"

class Channel;
class Poller;

class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    // 开启事件循环
    void loop();
    // 退出事件循环
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    // 在当前loop中执行cb
    void runInLoop(Functor cb);
    // 把cb放入队列中， 唤醒loop所在线程，执行cb
    void queueInLoop(Functor cb);

    // 唤醒loop所在线程的方法
    void wakeup();

    // EventLoop方法 --> Poller中的对应方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    // 判断EventLoop对象是否在自己的线程里
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
private:
    void handleRead();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel *>;

    // 运行状态
    std::atomic_bool looping_; // 原子操作， 通过CAS实现的
    std::atomic_bool quit_;

    // 当前loop的线程id
    const pid_t threadId_;
    // poller返回发生事件的时间点
    Timestamp pollReturnTime_;

    std::unique_ptr<Poller> poller_;

    // 主要作用，当mainLoop获取一个新用户的channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop处理channel
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    Channel *currentActiveChannel_;

    std::vector<Functor> pendingFunctor_;
    std::atomic_bool callingPendingFunctor_; // 标识当前loop是否有需要执行的回调操作
    std::mutex mutex_;                       // 互斥锁，用来保护上面的vector线程安全
};