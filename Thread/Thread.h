/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2023-12-18 09:17:17
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2023-12-18 15:11:49
 * @FilePath: /demo/reactorIO/Thread/Thread.h
 * @Description: 
 * @
 * @Copyright (c) 2023 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#ifndef REACTOR_THREAD_H
#define REACTOR_THREAD_H


#include <functional>
#include <thread>
#include <memory>
#include <string>
#include <atomic>

#include "../noncopyable.h"
#include "../easylogging++/easylogging++.h"


class Thread :noncopyable
{
public:
    // 线程工作函数
    using ThreadFunc = std::function<void()>;

    // 构造函数
    explicit Thread(ThreadFunc func, const std::string &name = std::string());
    ~Thread();

    void start();
    void join();

    bool status() { return m_status; }
    pid_t getTid() const { return m_tid; }

    const std::string& getThreadName() const { return m_threadName; }

    static int setAtomicThreadNum() { return m_threadNum; }

protected:

private:

    void setThreadName();

    bool m_status;
    bool m_join;

    std::shared_ptr<std::thread> m_thread;
    pid_t m_tid;
    ThreadFunc m_func;
    std::string m_threadName;
    static std::atomic<int> m_threadNum;

    std::thread::id m_threadId;
};

#endif // !REACTOR_THREAD_H