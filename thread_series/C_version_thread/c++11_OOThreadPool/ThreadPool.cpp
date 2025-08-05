#include "ThreadPool.hpp"
#include <memory>
// #include <chrono>
// #include <thread>

namespace ghz{

    ThreadPool::ThreadPool(int threadNum,int taskNum)
    :m_threadNum(threadNum)
    ,m_threadpool(m_threadNum)
    ,m_taskNum(taskNum)
    ,m_taskQueue(m_taskNum)
    ,m_isExit(false){
        
        //构造函数函数体(构造函数和析构函数的函数体基本上都是用来RAII的，其他的使用场景我暂时还想不到什么)
    }


    //一定是先有行为的思考，再有的函数的设计，然后最后才有的:决定是设计为木偶行为
    void ThreadPool::addTask(ElemType elem){
        m_taskQueue.push(elem);
    }
    ElemType ThreadPool::getTask(){
        return m_taskQueue.pop();
        //这里之所以不需要进行pop是否为空的判断，是因为在taskqueue这个类的pop函数中已经实现了对于队列空的判断了
        //所以还要有这种造了轮子就不需要反复进行健壮性判断了。这就是手动造轮子和使用抽象程度更高的库函数的使用相比，最大的优点
    }
    void ThreadPool::doTask(){

        while(!m_isExit){  //记住：线程池中的每个线程都是一个while循环，不断的取任务进行操作。直到完全没有任务了为止
        ElemType task=getTask();
        task->run();
        }
    }

    void ThreadPool::start(){
        //我认为代码逻辑应该是for循环中实现任务队列中的任务的启动
        //但是这里我又理解错了写法——————正确写法应该是回溯到thread文件中的启动函数的封装中，但是我想的是回溯到任务队列的启动了
        //所以回溯到调用哪个轮子错误，这也是个我需要抓紧解决的问题

        for(size_t idx=0;idx<m_threadNum;++idx){    //使用前向++，不要使用后向++(性能优化方式)
            //std::make_unique<Thread>(std::bind(&ThreadPool::doTask,this));
            //上面这种写法是基于对象的写法，别说我还真有点印象，印象深刻是因为可以不用写bind，直接写里面的参数即可。所以下午复习看看

            auto p_thread=std::make_unique<Thread>(workerthread(*this));
            //这样写肯定不对啊，因为没有进行workerthread的定义
            //所以以后写代码还是在clion上写，然后打开clion的提示，然后在测试的时候再放到vs上测试吧

            //这里如果写了workerthread的话，参数列表中传递的是使用的是哪个threadpool
            //所以这也是个非常常用的思想——————当设计类之间关系为依赖关系的时候，常常会设置为在参数列表中传这个类的指针。此时在设计的时候，基本上传的指针都是传this指针
            //所以归纳看那么多传this指针的情景，感觉基本上都是因为使用了依赖关系(以前传this指针的时候我都会非常费解为什么这里传this，现在应该就能明白了传this首先先看看是不是依赖关系)

            m_threadpool.emplace_back(p_thread);
        }

        for(auto thread:m_threadpool){
            
            thread->start();   //注意：这里调用的是线程的启动函数，而不是线程池的启动函数
        }
        //所以是两层for循环，而不是嵌套for循环
        //这个也想起来了，这些都应该非常熟练才对，再好好复习一遍
    }
    void ThreadPool::end(){

        while(!m_taskQueue.empty()){
            //std::this_thread::sleep_for(std::chrono::seconds(1));
            //这个函数可以常用，但是唯独不能在现在造轮子的位置使用——————因为这里造轮子是没有thread库的
            sleep(1);
        }

        m_isExit=true;

        //m_taskQueue.push(nullptr);   
        //纯属瞎提示——————要学会自己画类图然后自己分析——————即需要能分析出来end函数中就不可能有push和pop
        //所以我感觉很多时候没有思路，是因为我没有手绘类图(一定要手绘和UML同时进行，但是手绘绝对是最好的更好的提供思路的方法)
        
        for(auto thread:m_threadpool){
            thread->join();
        }

        //剩下的代码还没补全，原因是我对这些的标志位的健壮性完全没有考虑到
        //那这些标志位该如何想到呢？
        //我认为主要还是手机上写的全部列出来，然后要对每个标志位是干什么的有深刻理解
        //对每个标志位是干什么的有深刻理解了，就能知道什么时候该对标志位进行处理了
        //标志位代表着健壮性，为了健壮性，其他的可以不想，但是标志位一定一定要时刻能想到

        //m_isExit=true;    这个标志位设置为true还必须在所有join前进行，不然就一定会导致线程错误
        //所以所有的标志位都是需要严格控制顺序的，即每个多线程都需要首先考虑到


        weakup();
    }

    

    
}