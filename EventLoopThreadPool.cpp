/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-21 13:48:13
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-21 17:20:29
 * @FilePath: /studyMuduo/EventLoopThreadPool.cpp
 * @Description: #
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/

#include "EventLoopThreadPool.h"
#include "./easylogging++/easylogging++.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop, const std::string &name)
    : started_(false)
    , numThreads_(0)
    , next_(0)
    , name_(name)
    , baseLoop_(baseloop)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb)
{
    started_ = true;

    for (int i = 0; i < numThreads_; ++i)
    {
        std::string nameTmp = name_ + "_" + std::to_string(i);
        // LOG(INFO) << "Thread name " << nameTmp;
        EventLoopThread *t = new EventLoopThread(cb, nameTmp);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }

    if (numThreads_ == 0 && cb)
    {
        // LOG(INFO) << "EventLoopThreadPool::cb";
        cb(baseLoop_);
    } 
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    EventLoop *loop = baseLoop_;

    if (!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if (next_ >= loops_.size())
        {
            next_ = 0;
        }
    }

    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    if (loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}
