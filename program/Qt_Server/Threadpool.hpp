#ifndef __Threadpool_H__
#define __Threadpool_H__

#include "Task.hpp"
#include "TaskQueue.hpp"
#include "Thread.hpp"

#include <vector>
#include <memory>

using std::vector;
using std::unique_ptr;

namespace group_6
{


class Threadpool
{
public:
    Threadpool(int,int);

    void start();//开启线程池的运行
    void stop();//停止线程池的运行
    //Task是一个函数对象，用右值引用来表示
    void addTask(Task && cb);

    
    
private:
    //不是接口，放在了私有的区域
    void doTask();//每一个子线程都要做的事儿

private:
    vector<unique_ptr<Thread>>  _threads;
    int                         _threadNum;
    int                         _queSize;
    TaskQueue                   _taskque;
    bool                        _isExit;
};

}//end of namespace group_6

#endif

