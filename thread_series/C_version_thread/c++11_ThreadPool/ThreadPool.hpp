#include "Thread.hpp"
#include "Condition_Variable.hpp"
#include "Mutex.hpp"
#include "TaskQueue.hpp"

namespace ghz{

using ElemType=Task*;

class ThreadPool{
    public:   //对于这些权限问题，一定一定不要顶格写，一个TAB的长度就够了
     ThreadPool(size_t threadNum,size_t taskNum);
     ~ThreadPool()=default;

    void addTask(ElemType elem);
    ElemType getTask();
    void doTask();
    /*
    这些行为虽然是线程池行为，不过确实封装调用的消息队列的行为，
    这就是木偶行为和组合关系相配合造成的结果
    所以我认为木偶行为还可以进一步分解为和不同的模块间关系进行组合的时候，会迸发出什么样的效果？
    */

    //还有很多方法我忘记写了，这些忘记写的就是我对于线程池还不熟练的地方
    void start();
    void end();   //我自己改的名，用于实现结束所有的任务

    //注意：上面的都是对于线程池如何从任务队列里取内容，下面这些是进一步的，通过vector实现对于线程池的动态扩容优化
    //这个函数的设计是为了实现线程池的动态增长
    void addThread();
    //这个函数的设计是为了实现线程池的动态缩小
    void removeThread();
    //这个函数的设计是为了实现线程池的动态调整
    void adjustThreadNum(int num);



    private:
    //设计一个线程池放所有线程，设计一个消息队列
    size_t m_threadNum;   //这种取名方式非常非常好，直接了当标明到底是对于什么主语的数量
    //std::vector<Thread> m_threadPool;

    std::vector<std::unique_ptr<Thread>> m_threadPool;   
    //我之前写错了是因为我一直理解错了，对于线程池中存的只有是基类指针的时候，才能实现调用的时候多态
    //理解错误不要紧，这就是设计中先打草稿，然后再不断优化的过程。
    //所以必须要画出类图，画出来了才有草稿，才知道到时候自己需要修改哪些位置，否则就会不可能想到回溯到这个位置进行修改
    //但是需要主动记住：它和任务队列，一个是vector中存指针，一个是vecotr中存元素
    
    size_t m_taskNum;
    TaskQueue m_taskQueue;   
    //由于消息队列的结构已经封装了，所以就不用再使用vector了，而是直接使用封装后的数据结构即可
    //这何尝不是一种造轮子呢？所以所谓的造轮子，就是一级一级慢慢往上走，轮子和车都不是一步到位的，都是需要通过一步步组合才能实现的
    //并且这里的TaskQueue数据类型的对象是必须要使用int来初始化的，所以才必须有上面这个第三个数据成员

    bool m_isExit;    
    /*
    这个其实和线程造轮子的isRunning是一模一样的思想，所以这种对于线程判断和对于线程池判断的对比，要自己再进行一步归纳
    那分析：为什么线程问题这么迫切需要确定线程是否还在运行呢？
    因为线程是一个独立的执行流，所以我们需要一种机制来判断线程是否还在运行，这样我们才能知道线程是否还在运行
    */
};
}