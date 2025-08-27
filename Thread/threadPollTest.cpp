/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2023-12-19 16:58:11
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2023-12-21 14:32:22
 * @FilePath: /demo/studyMuduo/Thread/threadPollTest.cpp
 * @Description: 
 * @
 * @Copyright (c) 2023 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#include "ThreadPool.h"

INITIALIZE_EASYLOGGINGPP
// #define ELPP_THREAD_SAFE

int count = 1;

std::mutex threadMutex;
void print()
{
    std::cout << "------------print------------" << std::endl;
}

void print2()
{
    std::lock_guard<std::mutex> lock(threadMutex);
    // std::cout << count++ << std::endl;
    // LOG(INFO) << count++;
}

int main(int argc, char const *argv[])
{
    el::Configurations defaultConf;
    defaultConf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "true");
    defaultConf.set(el::Level::Global, el::ConfigurationType::Format, "[%level | %datetime | %file | %line | %msg]");
    defaultConf.setGlobally(el::ConfigurationType::Enabled, "true");
    el::Loggers::reconfigureLogger("default", defaultConf);

    // LOG(INFO) << "This is an informational message";
    // LOG(ERROR) << "This is an error message";

    ThreadPool threadpool;

    // threadpool.setThreadInitCallback(print);
    threadpool.setThreadPoolSize(1000);
    threadpool.start();

    for (size_t i = 0; i < 100000; i++)
    {
        threadpool.addThreadFuncQueue(print2);
    }

    // threadpool.addThreadFuncQueue([](){  sleep(3); });
    // threadpool.start();
    
    threadpool.stop();

    // LOG(INFO) << count;
    sleep(10);
    std::cout << "-----------------------------" << count << std::endl;
    return 0;
}
