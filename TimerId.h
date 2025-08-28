/********************************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2025-08-27 21:02:55
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2025-08-27 21:08:45
 * @FilePath: /my-muduo/TimerId.h
 * @Description:
 * @
 * @Copyright (c) 2025 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved.
 *********************************************************************/
#pragma once

#include "copyable.h"
#include <stdint.h>

class Timer;

// 一个不透明的标识符，用于取消定时器。
class TimerId : public copyable
{
public:
    // 默认构造函数，初始化timer_为NULL，sequence_为0
    TimerId() : timer_(nullptr),
                sequence_(0)
    {
    }

    // 构造函数，接受一个Timer指针和一个序列号
    TimerId(Timer *timer, uint64_t sequence)
        : timer_(timer),
          sequence_(sequence)
    {
    }

    // 默认拷贝构造函数、析构函数和赋值操作符是合适的

    friend class TimerQueue;

private:
    Timer *timer_;  // 定时器指针
    int64_t sequence_;  // 定时器序列号
}; //   class TimerId