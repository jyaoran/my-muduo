/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2023-12-18 10:23:16
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2023-12-20 09:18:28
 * @FilePath: /demo/reactorIO/Thread/Thread.cpp
 * @Description: 
 * @
 * @Copyright (c) 2023 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#include <semaphore.h>
#include "Thread.h"


std::atomic<int> Thread::m_threadNum(0);

Thread::Thread(ThreadFunc func, const std::string &name) 
    : m_func(std::move(func))
    , m_threadName(name)
    , m_status(false)
    , m_join(false)
    , m_tid(0)
{
    setThreadName();
}

Thread::~Thread()
{   
    // 线程运行并不处于阻塞状态时，对象进行析构时，需要分离线程使其完成业务后进行自行销毁
    if (m_status && (!m_join))
    {
        m_thread->detach();
    }   
}

void Thread::start()
{
    m_status = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    
    m_thread = std::shared_ptr<std::thread>(new std::thread(
        [&](){

            m_threadId = std::this_thread::get_id();

            // LOG(INFO) << "thread name : " << m_threadName << " create successful "  \
            // << " std::thread::id : " << m_threadId;
            sem_post(&sem);
            
            m_func();
    }));

    sem_wait(&sem);
}

void Thread::join()
{
    if (m_status == true)
    {
        m_join = true;
        m_thread->join();
    }
}

void Thread::setThreadName()
{
    ++m_threadNum;
    std::string countStr = std::to_string(m_threadNum.load());

    if (m_threadName.empty())
    {
        m_threadName = "Thread " + countStr;
    }
}
