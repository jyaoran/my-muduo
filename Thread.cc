/*
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-20 16:42:26
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-28 16:26:21
 * @FilePath: /studyMuduo/Thread.cc
 * @Description: 
 * 
 * Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 */
#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>

std::atomic_int Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false),
      joined_(false),
      tid_(0),
      func_(std::move(func)),
      name_(name)
{
    setDefualtName();
}

Thread::~Thread()
{
    if (started_ && !joined_)
    {
        thread_->detach();
    }
}

void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);

    thread_ = std::shared_ptr<std::thread>(new std::thread([&]()
                                                           {
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_(); }));

    sem_wait(&sem);
}

void Thread::join()
{
    this->joined_ = true;
    thread_->join();
}

void Thread::setDefualtName()
{
    int num = ++numCreated_;
    if (name_.empty())
    {
        name_ = name_ + std::to_string(num);
    }
}