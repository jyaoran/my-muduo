/********************************************************************
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2024-08-04 15:51:01
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2025-06-25 17:48:44
 * @FilePath: /studyMuduo/CurrentThread.cpp
 * @Description: 
 * @
 * @Copyright (c) 2025 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 *********************************************************************/
#include "CurrentThread.h"

namespace CurrentThread
{
    __thread int t_cachedTid = 0;

    void cachedTid()
    {
        if (t_cachedTid == 0)
        {
            // 此方法获取到的线程id与系统显示的一致
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}