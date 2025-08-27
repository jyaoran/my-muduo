/***********************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-22 10:01:13
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2024-02-22 13:01:36
 * @FilePath: /studyMuduo/Socket.cpp
 * @Description:
 * @
 * @Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved.
 ************************************************************/
#include "Socket.h"
#include "easylogging++/easylogging++.h"

#include <netinet/tcp.h>

Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localaddr)
{
    if (::bind(sockfd_, (sockaddr *)localaddr.getSockAddr(), sizeof(sockaddr_in)) != 0)
    {
        // LOG(ERROR) << "bind sockfd :" << sockfd_ << " fail. errno ----- : " << errno;
    }
}

void Socket::listen()
{
    if (::listen(sockfd_, 1024) != 0)
    {
        // LOG(ERROR) << "listen sockfd :" << sockfd_ << " fail.";
    }
}

int Socket::accept(InetAddress *peeraddr)
{
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    int connfd = accept4(sockfd_, (sockaddr *)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd >= 0)
    {
        peeraddr->setSockAddr(addr);
    }

    return connfd;
}

void Socket::shutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) < 0)
    {
        // LOG(ERROR) << "shutdownWrite error!";
    }
}

void Socket::setTcpNodelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}
