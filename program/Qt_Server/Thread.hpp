#ifndef __Thread_H__
#define __Thread_H__

#include "Noncopyable.hpp"
#include <pthread.h>

#include <functional>

namespace group_6
{
using ThreadCallback=std::function<void()>;

//具体类
class Thread : Noncopyable
{
public:
    //参数为右值引用,表示要绑定右值
    Thread(ThreadCallback && cb);
    ~Thread() {}

    void start();//启动一个子线程
    void join();//等待一个子线程运行结束

private:

    //子线程入口函数
    static void * start_routine(void*);

private:
    pthread_t      _pthid;//线程id
    bool           _isRunning;//线程的状态是否在运行
    ThreadCallback _cb;
};

}//end of namespace group_6


#endif

