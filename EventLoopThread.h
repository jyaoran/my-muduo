/********************************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-08-04 15:51:01
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2025-08-27 20:05:53
 * @FilePath: /my-muduo/EventLoopThread.h
 * @Description: 
 * @
 * @Copyright (c) 2025 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 *********************************************************************/

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
