#include "Thread.hpp"

#include <stdio.h>
#include <string.h>

//#include <iostream>

namespace wd
{

//cb本身是一个右值引用,但由于它是有名字的
//在构造函数中是左值, 为了表达完整的移动语义
//必须要使用std::move
Thread::Thread(ThreadCallback && cb)
: _pthid(0)
, _isRunning(false)
, _cb(std::move(cb))
{}

void Thread::start()
{
    if(!_isRunning) {
        int ret = pthread_create(&_pthid, 
                                 nullptr, 
                                 start_routine, 
                                 this);
        if(ret != 0) {
            fprintf(stderr, "%s", strerror(ret));
            return;
        }
        _isRunning = true;
    }
}


//这是一个静态的成员函数，没有this指针，
//无法直接获取到Thread的对象,因此应该通过
//线程入口函数的参数来传递
void * Thread::start_routine(void * arg)
{
    //在线程入口函数内部必须要调用run方法,
    //它代表的是线程要执行的任务
    Thread * pthread = static_cast<Thread*>(arg);
    if(pthread) {
        pthread->_cb();//基类指针调用纯虚函数
    }

    return nullptr;
}

void Thread::join()//等待一个子线程运行结束
{
    if(_isRunning) {
        pthread_join(_pthid, nullptr);

        _isRunning = false;//更改子线程的状态
    }
}

}//end of namespace wd
