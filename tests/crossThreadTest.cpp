/********************************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2025-06-25 16:43:18
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2025-06-25 17:10:17
 * @FilePath: /studyMuduo/lib/crossThreadTest.cpp
 * @Description: 
 * @
 * @Copyright (c) 2025 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 *********************************************************************/
#include <../EventLoop.h>
#include <../EventLoopThread.h>
#include <../Thread.h>
// #include <Logging.h>

#include <iostream>
#include <thread>

// using namespace muduo;
// using namespace muduo::net;

#include "easylogging++/easylogging++.h"

INITIALIZE_EASYLOGGINGPP


void printInLoopThread(const std::string& msg) {
    std::cout << "[Callback] running in thread: " << CurrentThread::tid() << ", msg = " << msg << std::endl;
}

int main() {
    std::cout << "[Main] thread id: " << CurrentThread::tid() << std::endl;

    // 创建一个独立线程运行 EventLoop
    EventLoopThread loopThread;
    EventLoop* loop = loopThread.startLoop();

    // 模拟在主线程向 loop 所属线程提交任务
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    loop->runInLoop(std::bind(printInLoopThread, "Hello from main thread"));

    Thread threadTest(std::bind(printInLoopThread, "Hello from thread_local"));
    threadTest.start();
    // 为了能观察现象，延迟退出
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "[Main] exiting" << std::endl;

    return 0;
}
