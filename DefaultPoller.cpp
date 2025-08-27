/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-01 14:54:05
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-19 15:16:12
 * @FilePath: /studyMuduo/DefaultPoller.cpp
 * @Description: 
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#include "Poller.h"
#include "EpollPoller.h"

#include <stdlib.h>


Poller *Poller::newDefaultPoller(EventLoop *loop)
{
    if (::getenv("MUDUO_USE_POLL"))
    {
        return nullptr; // 生成poll实例
    }
    else
    {
        return new EpollPoller(loop); // 生成epoll实例
    }
    
}