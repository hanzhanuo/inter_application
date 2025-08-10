#ifndef __TaskQueue_H__
#define __TaskQueue_H__

#include "Task.hpp"
#include "MutexLock.hpp"
#include "Condition.hpp"

#include <queue>
using std::queue;


namespace group_6
{

using ElemType = Task;

class TaskQueue
{
public:
    TaskQueue(int);
    bool empty() const;
    bool full() const;
    void push(ElemType);
    ElemType pop();

    void wakeup();

private:
    queue<ElemType>  _que;
    int         _queSize;
    MutexLock   _mutex;
    Condition   _notFull;
    Condition   _notEmpty;
    bool        _flag;
};


}//end of namespace group_6


#endif

