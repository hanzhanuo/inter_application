#include "TaskQueue.hpp"

namespace ghz{

    TaskQueue::TaskQueue(int size)
    :m_queSize(size){

    }

    void TaskQueue::push(ElemType elem){
        //对于标志位和锁，一定是先抢锁再抢标志位。否则就会导致白判断了——————这个问题我已经说过无数遍了
        
    }
    ElemType TaskQueue::pop(){
        
    }

    bool TaskQueue::isEmpty(){

    }
    bool TaskQueue::isFull(){

    }

    void TaskQueue::weakup(){
        notify_all();
        //标志位还没设计
    }
    
}