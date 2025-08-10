#include "MutexLock.hpp" //实现文件中第一个要对应的是自定义头文件
#include <pthread.h>

//#include <stdio.h>
//#include <iostream>

//using std::cout;
//using std::endl;


namespace wd
{

MutexLock::MutexLock()
{
    pthread_mutex_init(&_mutex, NULL);
}

MutexLock::~MutexLock()
{
    pthread_mutex_destroy(&_mutex);
}

void MutexLock::lock()
{
    pthread_mutex_lock(&_mutex);
}

void MutexLock::unlock()
{
    pthread_mutex_unlock(&_mutex);
}

}//end of namespace wd
