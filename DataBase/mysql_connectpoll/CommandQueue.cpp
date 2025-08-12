#include "CommandQueue.hpp"
#include <stdexcept>

namespace ghz{

    CommandQueue::CommandQueue(int queSize)
    : _queSize(queSize)
    , _mutex()
    //对两个条件变量设置锁参数
    , _notFull(_mutex)
    , _notEmpty(_mutex)
    , _flag(true) //默认允许操作
    {
        if(queSize <= 0) {
            throw std::invalid_argument("Queue size must be greater than zero.");
        }
    }
   

    //这里不能写为右值引用，因为这里是萝卜坑，不能进行
    void CommandQueue::push(ElemType cmd) {
        MutexLockGuard autolock(_mutex);
        //当队列已满时，需要等待
        while(full()) {
            _notFull.wait();
        }

        _que.push(cmd);

        //通知消费者线程取数据
        _notEmpty.notifyOne();
    }


    ElemType CommandQueue::pop() {
        MutexLockGuard autolock(_mutex);
        ElemType tmp;
        if(_que.empty()) {
            _notEmpty.wait();
        }
        if(_flag){
            //先获取队列首元素，然后才是pop操作
            tmp = _que.front();
            _que.pop();

            _notFull.notifyOne();  
        }else{
            tmp=nullptr;
            /*
            所以这里原本是可以直接写return nullptr的，
            但是为了return位置的统一，所以就直接在这个位置进行赋值，然后在最后再进行统一return了
            */
        }
        
        return tmp;
    }

    bool CommandQueue::empty() const {        
        return _que.empty();
    }

    bool CommandQueue::full() const{        
        return _que.size() == _queSize;
    }

    void CommandQueue::wakeup()
    {
         _flag = false;
        _notEmpty.notifyAll();
    }


}