/********************************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-08-04 15:51:01
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2025-08-27 20:07:27
 * @FilePath: /my-muduo/Thread.h
 * @Description: 
 * @
 * @Copyright (c) 2025 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 *********************************************************************/

#pragma once

#include "noncopyable.h"

#include <functional>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include <unistd.h>

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc func, const std::string &name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    int tid() const { return tid_; }
    const std::string &name() const { return name_; }

    static int numCreated() { return numCreated_; }

private:
    void setDefualtName();

    std::atomic_bool started_;
    std::atomic_bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    static std::atomic_int numCreated_;
};