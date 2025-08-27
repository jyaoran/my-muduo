/********************************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-08-04 15:51:01
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2025-06-25 17:23:43
 * @FilePath: /studyMuduo/EvnetLoopThreadPool.h
 * @Description: 
 * @
 * @Copyright (c) 2025 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 *********************************************************************/
#pragma once

#include"EventLoop.h"
#include "EventLoopThread.h"

#include <memory>
#include <functional>
#include <string>


class EventLoopThreadPool
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop *baseloop, const std::string &name);
    ~EventLoopThreadPool();

    void setThreadNumber(int numThreads) { numThreads_ = numThreads; }

    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    EventLoop *getNextLoop();

    std::vector<EventLoop*> getAllLoops();

    bool started() const { return started_; }

    const std::string name() const { return name_; }

private:
    EventLoop *baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};
