/***********************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-01-29 10:12:52
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-01-31 09:50:21
 * @FilePath: /demo/studyMuduo/Channel.cpp
 * @Description:
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved.
 ************************************************************/
#include "Channel.h"
#include "EventLoop.h"

#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd),
                                            events_(0), revents_(0), index_(-1), tied_(false)
{
}

Channel::~Channel()
{
}

// fd得到poller通知后需要处理的事件
void Channel::handleEvent(Timestamp receiveTime)
{
    if (tied_)
    {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
        
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

// channel的tie方法什么时候调用过？
// 一个TcpConnection新连接创建的时候，TcpConnetction => channel
void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;
    tied_ = true;
}

// 在channel所属的EventLoop中， 把当前channel移除。
void Channel::remove()
{
    // add func
    loop_->removeChannel(this);
}

// 当改变channel所表示fd的evnets事件后， update负责在poller里面更改fd相应的事件epoll_ctrl
// channel中无法加入poller成员，所以调用loop成员对象中的方法。
void Channel::update()
{    
    // 通过channel所属的EventLoop，调用poller的相应方法， 注册fd的events事件
    loop_->updateChannel(this);
}

/**
 *  EPOLLIN      // 可读事件
 *  EPOLLOUT     // 可写事件
 *  EPOLLRDHUP   // 对端关闭连接或半关闭连接
 *  EPOLLPRI     // 高优先级数据可读事件
 *  EPOLLERR     // 错误事件
 *  EPOLLHUP     // 挂起事件
 *  EPOLLET      // 边缘触发模式
 *  EPOLLONESHOT // 一次性事件
 */

// handleEvent()是Channel的核心，它根据revents_的值分别调用不同的用户回调
void Channel::handleEventWithGuard(Timestamp recvTime)
{
    // 判断是否发生的挂起事件并且没有可读事件
    // 挂起
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if (closeCallback_)
        {
            closeCallback_();
        }
    }

    // 对应的文件描述符发生错误， 调用error回调函数
    if (revents_ & EPOLLERR)
    {
        if (errorCallback_)
        {
            errorCallback_();
        }       
    }
    
    // 可读
    if (revents_ & EPOLLIN | EPOLLPRI | EPOLLRDHUP)
    {
        if (readCallback_)
        {
            readCallback_(recvTime);
        }        
    }
    
    // 可写
    if (revents_ & EPOLLOUT)
    {
        if (writeCallback_)
        {
            writeCallback_();
        }        
    }
}
