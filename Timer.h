/********************************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2025-08-27 20:09:09
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2025-08-28 21:45:14
 * @FilePath: /my-muduo/Timer.h
 * @Description: 
 * @
 * @Copyright (c) 2025 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 *********************************************************************/
#pragma once

#include <atomic>
#include "Timestamp.h"
#include "Callbacks.h"
#include "noncopyable.h"

#include <atomic>
#include "Timestamp.h"
#include "Callbacks.h"
#include "noncopyable.h"

// 定义Timer类，继承自noncopyable，确保该类对象不可复制
class Timer : noncopyable
{
 public:
  // Timer类的构造函数，接收一个回调函数、一个过期时间戳和一个间隔时间（以秒为单位）
  // 初始化成员变量：回调函数、过期时间戳、间隔时间、是否重复以及序列号
  Timer(TimerCallback cb, Timestamp when, double interval)
    : callback_(std::move(cb)),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0),
      sequence_(++s_numCreated_)
  { }

  // run方法，用于执行回调函数
  void run() const
  {
    callback_();
  }

  // 获取定时器的过期时间戳
  Timestamp expiration() const  { return expiration_; }

  // 检查定时器是否为重复定时器
  bool repeat() const { return repeat_; }

  // 获取定时器的序列号
  int64_t sequence() const { return sequence_; }

  // 重启定时器的方法
  void restart(Timestamp now);

  // 返回已创建的定时器的数量
  static int64_t numCreated() { return s_numCreated_.load(); }

 private:
  // 回调函数，当定时器到期时执行
  const TimerCallback callback_;

  // 定时器的过期时间戳
  Timestamp expiration_;

  // 定时器的时间间隔，单位为秒
  const double interval_;

  // 标记定时器是否为重复定时器
  const bool repeat_;

  // 定时器的序列号，用于唯一标识每个定时器
  const int64_t sequence_;

  // 静态原子变量，用于记录已创建的定时器数量
  static std::atomic<int64_t> s_numCreated_;
};
