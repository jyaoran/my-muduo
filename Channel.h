/***********************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-01-29 10:12:40
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-01-30 15:06:19
 * @FilePath: /demo/studyMuduo/Channel.h
 * @Description:
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved.
 ************************************************************/
#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

class EventLoop;
/**
 * channel 理解位通道
 * 封装了 socketfd 和 其感兴趣的事件event，如POLLIN、EPOLLIN事件
 * 还绑定了poller返回的具体事件。
 *
 */
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    // 处理事件 fd得到poller通知之后， 处理对应的事件。
    void handleEvent(Timestamp receiveTime);

    // 设置回调函数对象
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 将Channel与share_ptr管理的所有者对象绑定在一起，以防止在handleEvent执行期间销毁所有者对象。
    // 防止Channel被手动remove掉， channel还在执行回调函数
    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }

    int events() const { return events_; }

    void set_revents(int revt) { revents_ = revt; }

    bool isNoneEvent() const { return events_ == kNoneEvent; }

    // 注册fd感兴趣的事件
    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }
    void disableReading()
    {
        events_ &= ~kReadEvent;
        update();
    }
    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }
    void disableWriting()
    {
        events_ &= ~kWriteEvent;
        update();
    }
    void disableAll()
    {
        events_ = kNoneEvent;
        update();
    }
    bool isReading() { return events_ & kReadEvent; }
    bool isWriting() { return events_ & kWriteEvent; }

    // poller 相关的方法
    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }
    

    EventLoop *ownerLoop() { return loop_; }
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp recvTime);

    static const int kNoneEvent ;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_; // 事件循环
    const int fd_;    // fd, Poller监听的对象
    int events_;      // 注册fd感兴趣的事件
    int revents_;     // poller 返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};