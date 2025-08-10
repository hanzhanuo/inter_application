#ifndef __MutexLock_H__
#define __MutexLock_H__

//"" 双引号是从当前路径下进行查找
#include "Noncopyable.hpp"        //1. 自定义头文件

//<> 尖括号从系统路径之下进出查找
//#include <stdio.h>              //2. C的头文件  
#include <pthread.h>

//#include <iostream>             //3. C++的头文件
 
//#include <log4cpp/Category.hh>  //4. 第三方库头文件


namespace wd
{


class MutexLock : Noncopyable
{
public:
    MutexLock();
    ~MutexLock();

    void lock();
    void unlock();
    pthread_mutex_t * getMutexLockPtr() 
    {   return &_mutex; }

//protected:

//私有的数据成员要放在类的最下面
private:
    pthread_mutex_t _mutex;
};


class MutexLockGuard
{
public:
    MutexLockGuard(MutexLock & m)
    : _mutex(m)
    {
        _mutex.lock();
    }

    ~MutexLockGuard()
    {
        _mutex.unlock();
    }
private:
    MutexLock & _mutex;

};

}//end of namespace wd


#endif

