/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2023-12-14 13:16:51
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2023-12-20 10:17:00
 * @FilePath: /demo/studyMuduo/noncopyable.h
 * @Description: 
 * @
 * @Copyright (c) 2023 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#ifndef REACTOR_NONCOPYABLE_H
#define REACTOR_NONCOPYABLE_H



class noncopyable
{
 public:
  noncopyable(const noncopyable&) = delete;
  void operator=(const noncopyable&) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

#endif  // REACTOR_NONCOPYABLE_H
