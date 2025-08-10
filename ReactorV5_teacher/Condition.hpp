#ifndef __Condition_H__
#define __Condition_H__

#include "Noncopyable.hpp"

#include <pthread.h>

namespace wd
{

class MutexLock;//类的前向声明, 防止出现头文件的循环依赖

class Condition : Noncopyable
{
public:
    Condition(MutexLock &);
    ~Condition();

    void wait();
    void notifyOne();
    void notifyAll();

private:
    pthread_cond_t _cond;
    MutexLock & _mutex;//引用占据的空间是一个指针的大小
};

}//end of namespace wd


#endif

