/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-29 10:33:56
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-29 10:44:39
 * @FilePath: /studyMuduo/lib/test.cpp
 * @Description: 
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#include "easylogging++.h"


INITIALIZE_EASYLOGGINGPP

int main(int argc, char const *argv[])
{

    el::Configurations conf("../my-conf.conf");
    el::Loggers::reconfigureLogger("default", conf);
    el::Loggers::reconfigureAllLoggers(conf);


    LOG(INFO) << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
    LOG(INFO) << "日志信息";
    LOG(ERROR) << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
    LOG(ERROR) << "错误信息";
    // LOG(FATAL) << "epoll_ctl fatal! errno =";

    
    
    return 0;
}

