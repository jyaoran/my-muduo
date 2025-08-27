/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-01 15:14:50
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-29 10:41:00
 * @FilePath: /studyMuduo/EpollPoller.cpp
 * @Description: 
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#include "EpollPoller.h"
#include "easylogging++/easylogging++.h"

#include <string.h>

// Channel未添加到poller中
const int kNew = -1;
// Channel 已添加到poller中
const int kAdded = 1;
// channel 从poller中删除
const int kDelete = 2;

EpollPoller::EpollPoller(EventLoop *loop) : Poller(loop), events_(kInitEvnetListSize)
{
    epollfd_ = ::epoll_create1(EPOLL_CLOEXEC);
    if (epollfd_ < 0)
    {
        // LOG(ERROR) << "epoll_create error : " << errno;
        exit(-1); 
    }
    

}

EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}

Timestamp EpollPoller::poll(int timeOutMs, ChannelList *activeChannels)
{
    int numEvents;
    int saveErrno;
    
    numEvents = ::epoll_wait(epollfd_, &(*events_.begin()), static_cast<int>(events_.size()), timeOutMs);
    // 调用完epoll后立即保存errno， 防止其他系统调用修改errno。
    saveErrno = errno;

    Timestamp now(Timestamp::now());

    if (numEvents > 0)  
    {
        fillActivChannels(numEvents, activeChannels);
        if (numEvents == events_.size())
        {
            events_.resize(events_.size()*2);
        }
        else if (numEvents == 0)
        {
            //debug
            // LOG(DEBUG) << "nothing happened";
        }
        else
        {
            if (saveErrno != EINTR)
            {
                // saveErrno
                // LOG(ERROR) << "sysError EPollPoller::poll() errno : " << saveErrno;
            }            
        }        
    }

    return now;
}

void EpollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();

    if (index == kNew || index == kDelete)
    {
        // 一个新的channel， add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew)
        {
            m_channels[fd] = channel;
        }
        
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDelete);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    int index = channel->index();

    m_channels.erase(fd);
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    
    channel->set_index(kNew);
}

void EpollPoller::fillActivChannels(int numEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numEvents; i++)
    {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EpollPoller::update(int operation, Channel *channel)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));

    event.events = channel->events();
    event.data.ptr = channel;

    int fd = channel->fd();

    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            // LOG(ERROR) << "epoll_ctl error! errno = " << errno; 
        }
        else
        {
            // LOG(FATAL) << "epoll_ctl fatal! errno = " << errno;
        }
    }
    
}
