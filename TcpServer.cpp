/***********************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-18 14:15:40
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-28 15:06:01
 * @FilePath: /studyMuduo/TcpServer.cpp
 * @Description:
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved.
 ************************************************************/
#include "TcpServer.h"
#include "TcpConnection.h"
#include "easylogging++/easylogging++.h"

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        // LOG(ERROR) << "mainLoop is null!";
    }

    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option)
    : loop_(CheckLoopNotNull(loop)), ipPort_(listenAddr.toIpPort()),
      name_(nameArg), acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, nameArg)), connectionCallback_(),
      messageCallback_(), writeCompleteCallback_(), threadInitCallback_(), started_(0), nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    for (auto &item : connections_)
    {
        TcpConnectionPtr conn(item.second);

        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int threadNum)
{
    threadPool_->setThreadNumber(threadNum);
}

void TcpServer::start()
{
    if (started_++ == 0)
    {
        threadPool_->start(threadInitCallback_);

        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
    
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    EventLoop *ioLoop = threadPool_->getNextLoop();

    std::string connName = name_ + "--" + ipPort_ + "##" + std::to_string(nextConnId_++);

    // LOG(INFO) << "TcpServer::newConnection " << connName;

    sockaddr_in local;
    socklen_t addrlen = sizeof(local);
    ::memset(&local, 0, addrlen);

    if (::getsockname(sockfd, (sockaddr *)&local, &addrlen))
    {
        // LOG(ERROR) << "::getsockname() error!";
    }
    
    InetAddress localAddr(local);
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    loop_->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
