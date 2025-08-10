#include "Threadpool.hpp"

#include <unistd.h>
#include <stdio.h>


namespace group_6
{

Threadpool::Threadpool(int threadNum, int queSize)
: _threads() //vector的无参构造函数没有开辟空间
, _threadNum(threadNum)
, _queSize(queSize)
, _taskque(_queSize)
, _isExit(false)
{
    //提前准备好空间了, 但还没有元素,size（）为0
    _threads.reserve(_threadNum);
}

void Threadpool::start()
{
    for(int i = 0; i < _threadNum; ++i) {
        //创建N个子线程对象WorkerThread
        //将线程池的doTask方法注册给线程对象
        unique_ptr<Thread> up(new Thread(std::bind(&Threadpool::doTask, this)));
        //存入数组中, up是一个左值,必须要转换成右值，
        //才能转移到容器中
        _threads.push_back(std::move(up));
    }

    //让每一个子线程运行起来
    for(auto & pthread : _threads) {
        pthread->start();
    }
}

void Threadpool::addTask(Task && task)
{
    if(task) {
        _taskque.push(std::move(task));
    }
}
    

void Threadpool::stop()//停止线程池的运行
{
    //当任务队列中还有任务时，暂时不退出，等待任务执行完毕
    while(!_taskque.empty()) {
        sleep(1);
    }

    //修改退出的标志位
    _isExit = true;

    //通知所有的子线程，要从wait中醒来
    _taskque.wakeup();

    printf("\nthreadpool is starting join sub thread\n");
    //要回收每一个子线程
    for(auto & pthread : _threads) {
        pthread->join();
    }
}

void Threadpool::doTask()
{
    //当子线程退出while循环时，子线程就会结束运行
    while(!_isExit) {
        //每一个子线程都要从任务队列中获取任务，并执行
        Task task = _taskque.pop();
        if(task) {
            //执行任务
            task();
            //bug: 当子线程的任务执行的速度过快，
            //在stop方法还没有来得及将_isExit设置为true
            //之前，子线程又进入下一次循环，阻塞在
            //pop方法之上
        }
    }
}


}// end of namespace group_6


