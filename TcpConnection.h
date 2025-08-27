/***********************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-27 09:23:55
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-27 11:09:26
 * @FilePath: /studyMuduo/TcpConnection.h
 * @Description:
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved.
 ************************************************************/
#pragma once

#include <memory>
#include <atomic>
#include <string>

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Timestamp.h"
#include "Buffer.h"

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop, const std::string &nameArg, int sockfd,
                  const InetAddress &localAddr, const InetAddress &peerAddr);

    ~TcpConnection();

    EventLoop *getLoop() const { return loop_; }
    const std::string &name() const { return name_; }
    const InetAddress &localAddress() const { return localAddr_; }
    const InetAddress &peerAddress() const {return peerAddr_; }

    bool connected() const { return state_ == kConnected; }

    // 发送数据
    void send(const std::string &buf);
    void send(Buffer *buf);

    // 关闭连接
    void shutdown();

    // 保存用户自定义的回调函数
    void setConnectionCallback(ConnectionCallback cb)
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(MessageCallback cb)
    {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(WriteCompleteCallback cb)
    {
        writeCompleteCallback_ = cb;
    }

    void setCloseCallback(CloseCallback cb)
    {
        closeCallback_ = cb;
    }

    void setHighWaterMarkCallback(HighWaterMarkCallback cb, ssize_t highWaterMark)
    {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    // TcpServer 会调用
    void connectEstablished();  // 确定建立连接
    void connectDestroyed();    // 连接销毁
private:
    enum StateE
    {
        kDisconnected,  // 已经断开连接
        kConnecting,    // 正在连接
        kConnected,     // 已连接
        kDisconnecting  // 正在断开连接
    };

    void setState(StateE state) { state_ = state; }

    // 注册到Channel上的回调函数， poller通知后会调用这些函数处理
    // 然后这些函数最后会在调用从用户哪里传进来的回调函数。
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void *message, size_t len);
    void sendInLoop(const std::string &message);
    void shutdownInLoop();

    EventLoop *loop_; // 这里绝对不是baseLoop
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_; // 本地地址
    const InetAddress peerAddr_;  // 对端地址

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;
    HighWaterMarkCallback highWaterMarkCallback_; // 超出水位实现的回调
    ssize_t highWaterMark_;                        // 水位值

    Buffer inputBuffer_;  // 读取数据的缓冲区
    Buffer outputBuffer_; // 发送数据的缓冲区
};