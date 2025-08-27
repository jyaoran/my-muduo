/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2023-12-19 10:53:07
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-29 10:43:29
 * @FilePath: /studyMuduo/Thread/ThreadPool.cpp
 * @Description: 
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#include "ThreadPool.h"

ThreadPool::ThreadPool(const std::string &name)
    : m_name(name), m_running(false), m_mutex(), m_cond(), m_threadSize(INIT_THREAD_SIZE)
{
}

ThreadPool::~ThreadPool()
{
    stop();
    for (const auto &t : m_threads)
    {
        // 阻塞等待线程运行结束
        t->join();
    }
}

void ThreadPool::start()
{
    m_running = true;
    m_threads.reserve(m_threadSize);
    size_t i = 0;

    for (i = 0; i < m_threadSize; i++)
    {
        std::string id = std::to_string(i + 1);
        m_threads.emplace_back(new Thread(std::bind(&ThreadPool::runInThread, this), m_name + id));
        m_threads[i]->start();
    }

    if (m_threadSize == 0 && m_threadInitCallback)
    {
        m_threadInitCallback();
    }
}

void ThreadPool::stop()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_running = false;
    m_cond.notify_all();
}

size_t ThreadPool::getQueueSize() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_funcDeque.size();
}

void ThreadPool::addThreadFuncQueue(ThreadFunction ThreadFunction)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_funcDeque.push_back(ThreadFunction);
    m_cond.notify_one();
}

void ThreadPool::runInThread()
{
    try
    {
        if (m_threadInitCallback)
        {
            m_threadInitCallback();
        }

        ThreadFunction task;

        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                while (m_funcDeque.empty())
                {
                    if (m_running == false)
                    {
                        return;
                    }
                    m_cond.wait(lock);
                }
                task = m_funcDeque.front();
                m_funcDeque.pop_front();
            }

            if (task != nullptr)
            {
                task();
            }
        }
    }
    catch (...)
    {
        // LOG(ERROR) << "runInThread throw exception";
    }
}
