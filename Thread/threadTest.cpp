/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2023-12-18 14:48:18
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2023-12-20 08:52:22
 * @FilePath: /demo/reactorIO/Thread/threadTest.cpp
 * @Description: 
 * @
 * @Copyright (c) 2023 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#include <iostream>
#include <mutex>
#include <string>
#include "Thread.h"

INITIALIZE_EASYLOGGINGPP

std::mutex mainMutex;


void print(std::string &id)
{
    // std::lock_guard<std::mutex> lock(mainMutex);
    // LOG(INFO) << "into thread i : " << id;
}

int main(int argc, char const *argv[])
{
    size_t i = 0;
    std::string threadName = "thread";
    std::vector<std::unique_ptr<Thread>> threads;

    el::Configurations defaultConf;
    defaultConf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "true");
    defaultConf.set(el::Level::Global, el::ConfigurationType::Format, "[%level | %datetime | %file | %line | %msg]");
    defaultConf.setGlobally(el::ConfigurationType::Enabled, "true");
    el::Loggers::reconfigureLogger("default", defaultConf);

    // LOG(INFO) << "This is an informational message";
    // LOG(ERROR) << "This is an error message";

    for (i = 0; i < 100; i++)
    {
        // std::lock_guard<std::mutex> lock(mainMutex);
        std::string id = std::to_string(i+1);
        threads.emplace_back(new Thread(std::bind(print, id)));

        threads[i]->start();
    }
    
    for (auto &thread_is : threads)
    {
        thread_is->join();
    }

    return 0;
}

