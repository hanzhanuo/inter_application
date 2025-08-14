#include "MutexLock.hpp" //实现文件中第一个要对应的是自定义头文件





namespace group_6
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

}//end of namespace group_6
