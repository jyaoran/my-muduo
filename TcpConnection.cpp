/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-27 17:27:21
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-29 10:42:43
 * @FilePath: /studyMuduo/TcpConnection.cpp
 * @Description: 
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
/***********************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-27 09:24:10
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-28 10:13:44
 * @FilePath: /studyMuduo/TcpConnection.cpp
 * @Description:
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved.
 ************************************************************/
#include "TcpConnection.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "easylogging++/easylogging++.h"

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        // LOG(ERROR) << "mainLoop is null!";
    }

    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, const std::string &nameArg, int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr)
    : loop_(CheckLoopNotNull(loop)), name_(nameArg), state_(kConnecting), socket_(new Socket(sockfd)), channel_(new Channel(loop, sockfd)), localAddr_(localAddr), peerAddr_(peerAddr), highWaterMark_(64 * 1024 * 1024)
{
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));

    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));

    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));

    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::send(const std::string &buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf.c_str(), buf.size());
        }
        else
        {
            // 遇到重载函数的绑定，可以使用函数指针来指定确切的函数
            void (TcpConnection::*fp)(const void *message, size_t len) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::send(Buffer *buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            // sendInLoop有多重重载，需要使用函数指针确定
            void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()));
        }
        
    }
    
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
    
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);

    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int saveErrno = 0;

    size_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        // LOG(ERROR) << "TcpConnection::handleRead() failed! errno : " << saveErrno;
        handleError();
    }   
}

void TcpConnection::handleWrite()
{
    if (channel_->isWriting())
    {
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this())); 
                }
                
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                } 
            }            
        }      
    }
    else
    {
        // LOG(ERROR) << "TcpConnection::handleWrite() failed!";
    }
    
}

void TcpConnection::handleClose()
{
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);
    closeCallback_(connPtr);
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof(optval);
    int err = 0;

    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen))
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    
}

void TcpConnection::sendInLoop(const void *message, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if (state_ == kDisconnected)
    {
        return;
    }

    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), message, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_)
            {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else    // nwrote < 0  
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }
    
    if (!faultError && remaining > 0)
    {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
        {
             loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        
        outputBuffer_.append((char *)message + nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableReading();
        }
    } 
}

void TcpConnection::sendInLoop(const std::string &message)
{
    sendInLoop(message.c_str(), message.size());
}

void TcpConnection::shutdownInLoop()
{
    if (!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
    
}
