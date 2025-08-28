/*
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-02-28 16:27:36
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2025-06-25 17:15:27
 * @FilePath: /studyMuduo/lib/echoServer.cc
 * @Description: 
 * 
 * Copyright (c) 2024 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 */

#include "../EventLoop.h"
#include "../TcpServer.h"
#include "../Callbacks.h"

#include "easylogging++/easylogging++.h"

INITIALIZE_EASYLOGGINGPP


class EchoServer
{
public:
    EchoServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
        : loop_(loop)
        , server_(loop, addr, name)
    {
        // 注册回调函数
        server_.setConnectionCallback(
            std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        
        server_.setMessageCallback(
            std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        // 设置合适的subloop线程数量
        server_.setThreadNum(4);
    }

    void start()
    {
        server_.start();
    }

private:
    // 连接建立或断开的回调函数
    void onConnection(const TcpConnectionPtr &conn)   
    {
        if (conn->connected())
        {
            LOG(INFO) << "Connection UP : " << conn->peerAddress().toIpPort().c_str();
        }
        else
        {
            LOG(INFO) << "Connection DOWN : " << conn->peerAddress().toIpPort().c_str();
        }
    }

    // 可读写事件回调
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
    {
        std::string msg = buf->retrieveAllAsString();
        
		LOG(INFO) << conn->name() << " echo " << msg.size() << " bytes, "
                 << "data received at " << time.toFormattedString();
        
		conn->send(msg);
        
		LOG(INFO) << msg;
        
		// conn->shutdown();   // 关闭写端 底层响应EPOLLHUP => 执行closeCallback_
    }

    EventLoop *loop_;
    TcpServer server_;
};

int main() 
{
    el::Configurations conf("../my-conf.conf");
    el::Loggers::reconfigureLogger("default", conf);
    el::Loggers::reconfigureAllLoggers(conf);


    LOG(INFO) << "pid = " << getpid();
    // LOG(INFO) << "pid = ";
    // LOG(FATAL) << "epoll_ctl fatal! errno =";


	EventLoop loop;
    InetAddress addr(8080);
    EchoServer server(&loop, addr, "EchoServer");
    
    server.start();
    loop.loop();

    return 0;
}
