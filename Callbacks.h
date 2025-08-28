/********************************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-08-04 15:51:01
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2025-08-27 20:15:10
 * @FilePath: /my-muduo/Callbacks.h
 * @Description: 
 * @
 * @Copyright (c) 2025 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 *********************************************************************/

#pragma once

#include <memory>
#include <functional>

class Buffer;
class TcpConnection;
class Timestamp;

using TcpConnectionPtr   =    std::shared_ptr<TcpConnection>;
using ConnectionCallback =    std::function<void(const TcpConnectionPtr &)>;
using CloseCallback      =    std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback       = std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;

using TimerCallback = std::function<void()>;