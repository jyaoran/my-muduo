/*********************************************************** 
 * @Author: jiangshan yaoranyaoran2015@outlook.com
 * @Date: 2023-12-20 10:12:20
 * @LastEditors: jiangshan yaoranyaoran2015@outlook.com
 * @LastEditTime: 2023-12-20 10:42:20
 * @FilePath: /demo/studyMuduo/Thread/ThreadLocalStorage.h
 * @Description: 
 * @
 * @Copyright (c) 2023 by jiangshan yaoranyaoran2015@outlook.com, All Rights Reserved. 
 ************************************************************/
#ifndef REACTOR_THREAD_LOCAL_STORAGE_H
#define REACTOR_THREAD_LOCAL_STORAGE_H

#include "noncopyable.h"

#include <pthread.h>

/**
 * 这是一个简化的线程局部存储（Thread Local Storage，TLS）的实现，
 * 使用了 C++11 中的 pthread_key_create 和 pthread_setspecific 函数。
 * 线程局部存储允许每个线程拥有自己的变量实例，而不同线程之间互不影响。
 */

template<class T>
class ThreadLocalStorage : noncopyable
{
public:
    ThreadLocalStorage()
    {
        // 使用 pthread_key_create 创建了一个线程局部存储键（key_），
        // 并指定了一个析构函数 ThreadLocal::destructor
        pthread_key_create(&m_pkey, &ThreadLocalStorage::deletePerThreadValue);
    }

    ~ThreadLocalStorage()
    {
        // 通过 pthread_key_delete 删除了线程局部存储键
        pthread_key_delete(m_pkey);
    }

    T &value()
    {
        // 通过pthread_getspecific(m_pkey)获取到对应的局部变量
        T *perThreadValue = static_cast<T*>pthread_getspecific(m_pkey);
        if (perThreadValue == nullptr)
        {
            // 局部变量不存在， new一个变量，添加到m_pkey中
            T *obj = new T();
            pthread_setspecific(m_pkey, obj);
            perThreadValue = obj;
        }
        
        return *perThreadValue;
    }
protected:

private:

    // 释放对应的变量的析构函数
    static void deletePerThreadValue(void *value)
    {
        T *obj = static_cast<T*>(value);
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy; (void) dummy;
        delete obj;
    }

    pthread_key_t m_pkey;
};


#endif // !REACTOR_THREAD_LOCAL_STORAGE_H
