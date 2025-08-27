/********************************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2025-06-25 17:17:34
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2025-06-25 17:25:18
 * @FilePath: /studyMuduo/lib/eventLoopThread_unitTest.cpp
 * @Description: 
 * @
 * @Copyright (c) 2025 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 *********************************************************************/
#include <../EventLoop.h>
#include <../EventLoopThread.h>
#include <../EventLoopThreadPool.h>
#include <../Thread.h>
// #include <Logging.h>

#include <iostream>
#include <thread>
#include <assert.h>

// using namespace muduo;
// using namespace muduo::net;

#include "easylogging++/easylogging++.h"

INITIALIZE_EASYLOGGINGPP


 void print(EventLoop* p = NULL)
{
  printf("main(): pid = %d, tid = %d, loop = %p\n",
         getpid(), CurrentThread::tid(), p);
}

void init(EventLoop* p)
{
  printf("init(): pid = %d, tid = %d, loop = %p\n",
         getpid(), CurrentThread::tid(), p);
}

int main()
{
  print();

  EventLoop loop;
//   loop.runAfter(11, std::bind(&EventLoop::quit, &loop));

  {
    printf("Single thread %p:\n", &loop);
    EventLoopThreadPool model(&loop, "single");
    model.setThreadNumber(0);
    model.start(init);
    assert(model.getNextLoop() == &loop);
    assert(model.getNextLoop() == &loop);
    assert(model.getNextLoop() == &loop);
  }

  {
    printf("Another thread:\n");
    EventLoopThreadPool model(&loop, "another");
    model.setThreadNumber(1);
    model.start(init);
    EventLoop* nextLoop = model.getNextLoop();
    // nextLoop->runAfter(2, std::bind(print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop == model.getNextLoop());
    assert(nextLoop == model.getNextLoop());
    ::sleep(3);
  }

  {
    printf("Three threads:\n");
    EventLoopThreadPool model(&loop, "three");
    model.setThreadNumber(3);
    model.start(init);
    EventLoop* nextLoop = model.getNextLoop();
    nextLoop->runInLoop(std::bind(print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop != model.getNextLoop());
    assert(nextLoop != model.getNextLoop());
    assert(nextLoop == model.getNextLoop());
  }

  loop.loop();
}