/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-21 13:48:43
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-21 17:09:05
 * @FilePath: /studyMuduo/EventLoopThread.cpp
 * @Description: 
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#include "EventLoopThread.h"
#include "./easylogging++/easylogging++.h"


EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
    : exite_(false)
    , loop_(nullptr)
    , thread_(std::bind(&EventLoopThread::threadFunc, this), name)
    , callback_(cb)
{

}

EventLoopThread::~EventLoopThread()
{
    exite_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
    
}

EventLoop *EventLoopThread::startLoop()
{
    thread_.start();
    EventLoop *loop = nullptr;
    std::unique_lock<std::mutex> lock(mutex_);
    while (loop_ == nullptr)
    {
        cond_.wait(lock);
    }
    
    loop = loop_;
    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    
    if (callback_)
    {
        // LOG(INFO) << "EventLoopThread::callback_";
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}
