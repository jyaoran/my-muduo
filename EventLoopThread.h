/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-21 10:30:33
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-21 13:50:44
 * @FilePath: /studyMuduo/EventLoopThread.h
 * @Description: 
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#pragma once

#include "noncopyable.h" 
#include "Thread.h"
#include "EventLoop.h"

#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>


class EventLoop;

class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    
    explicit EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), 
        const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop *startLoop();

private:
    void threadFunc();

    bool exite_;
    EventLoop *loop_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};
