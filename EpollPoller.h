/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2023-12-14 13:12:39
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-20 16:25:44
 * @FilePath: /studyMuduo/EpollPoller.h
 * @Description: 
 * @
 * @Copyright (c) 2023 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#ifndef REACTOR_EPOLL_H
#define REACTOR_EPOLL_H

#include "Poller.h"
#include "Timestamp.h"

#include <vector>

#include <unistd.h>
#include <assert.h>
#include <sys/epoll.h>
#include <error.h>

/*
    EPOLLIN      // 可读事件
    EPOLLOUT     // 可写事件
    EPOLLRDHUP   // 对端关闭连接或半关闭连接
    EPOLLPRI     // 高优先级数据可读事件
    EPOLLERR     // 错误事件
    EPOLLHUP     // 挂起事件
    EPOLLET      // 边缘触发模式
    EPOLLONESHOT // 一次性事件
*/

class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;

    Timestamp poll(int timeOutMs, ChannelList *activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;

protected:

private:
    static const int kInitEvnetListSize = 16;

    // 填充活跃fd事件的channel
    void fillActivChannels(int numEvents, ChannelList *activeChannels) const;

    void update(int operation, Channel *channel);
    
    using EventList = std::vector<struct epoll_event>;
    int epollfd_;
    EventList events_;
};

#endif // !REACTOR_EPOLL_H
