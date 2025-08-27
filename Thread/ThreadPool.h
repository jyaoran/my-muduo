/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2023-12-19 10:52:56
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2023-12-20 10:16:27
 * @FilePath: /demo/studyMuduo/Thread/ThreadPool.h
 * @Description: 
 * @
 * @Copyright (c) 2023 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#ifndef REACTOR_THREADPOOL_H
#define REACTOR_THREADPOOL_H

#include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "../noncopyable.h"
#include "Thread.h"
#include "../easylogging++/easylogging++.h"

class ThreadPool : noncopyable
{
public:
    using ThreadFunction = std::function<void()>;
#define INIT_THREAD_SIZE 10
    explicit ThreadPool(const std::string &name = std::string("ThreadPool-"));
    ~ThreadPool();

    void setThreadInitCallback(const ThreadFunction &callbackFunc) 
    { 
        m_threadInitCallback = callbackFunc;
    }

    void setThreadPoolSize(const size_t &size)
    {
        m_threadSize = size;
    }

    void start();
    void stop();

    const std::string &getThreadName() const
    {
        return m_name;
    }

    size_t getQueueSize() const;

    void addThreadFuncQueue(ThreadFunction ThreadFunction);

protected:


private:

    void runInThread();

    using ThreadPtrVec = std::vector<std::unique_ptr<Thread>>;
    using ThreadFuncDeque = std::deque<ThreadFunction>;

    ThreadFunction m_threadInitCallback;
    std::string m_name;
    size_t m_threadSize;
    std::condition_variable m_cond;
    mutable std::mutex m_mutex;
    ThreadPtrVec m_threads;
    ThreadFuncDeque m_funcDeque;
    bool m_running;

};

#endif // !REACTOR_THREADPOOL_H