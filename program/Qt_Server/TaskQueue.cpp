#include "TaskQueue.hpp"
#include "MutexLock.hpp"


namespace group_6
{

TaskQueue::TaskQueue(int queSize)
: _que() 
, _queSize(queSize)
, _mutex()
, _notFull(_mutex)
, _notEmpty(_mutex)
, _flag(true)
{}

bool TaskQueue::empty() const
{
    return _que.size() == 0;
}

bool TaskQueue::full() const
{
    return _que.size() == _queSize;
}

//push是运行在生产者线程
void TaskQueue::push(ElemType e)
{
    ///autolock是一个局部变量
    MutexLockGuard autolock(_mutex);
    //当队列已满时，需要等待
    while(full()) {
        _notFull.wait();
    }

    _que.push(e);

    //通知消费者线程取数据
    _notEmpty.notifyOne();
}

//pop是运行在消费者线程
ElemType TaskQueue::pop()
{
    ElemType tmp;
    MutexLockGuard autolock(_mutex);
    while(_flag && empty()) {
        _notEmpty.wait();
    }

    if(_flag) {
        tmp = _que.front();
        _que.pop();
        //通知生产者线程放数据
        _notFull.notifyOne();
    } else {
        tmp = nullptr;
    }

    return tmp;
}

void TaskQueue::wakeup()
{
    _flag = false;
    _notEmpty.notifyAll();
}

}//end of namespace group_6
