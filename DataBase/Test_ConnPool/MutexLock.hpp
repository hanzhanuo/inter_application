#ifndef __MutexLock_H__
#define __MutexLock_H__

#include "Noncopyable.hpp"    
#include <pthread.h>

namespace group_6
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

}//end of namespace group_6


#endif

