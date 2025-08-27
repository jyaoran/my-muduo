/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2023-12-14 13:22:44
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-01 10:03:53
 * @FilePath: /demo/studyMuduo/Poller.h
 * @Description: 一个虚基类，统一轮询接口
 * @
 * @Copyright (c) 2023 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#ifndef REACTOR_POLLER_H
#define REACTOR_POLLER_H

#include <map>
#include <vector>

#include "Timestamp.h"
#include "noncopyable.h"
#include "Channel.h"



class EventLoop;
class Channel;
/**
 * moduo 中多路事件分发器的核心IO复用模块
*/
class Poller : noncopyable  // class 默认继承权限为 private
{
public:
    using ChannelList = std::vector<Channel*>;
    Poller(EventLoop *loop) : m_ownerLoop(loop) {}
    virtual ~Poller() = default;

    // 给所有IO服用操作保留统一的接口
    virtual Timestamp poll(int timeOutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *Channel) = 0;

    // channel是否在此循环中
    bool hasChannel(Channel *Channel) const
    {
        auto it = m_channels.find(Channel->fd());
        return (it != m_channels.end() && it->second == Channel);
    }

    // EventLoop 可以通过该接口获取默认的IO服用的具体实现
    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    using ChannelMap = std::map<int, Channel*>;
    ChannelMap m_channels;

private:
    EventLoop *m_ownerLoop;
};

#endif // !REACTOR_POLLER_H
