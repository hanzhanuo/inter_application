#include "Thread.hpp"
#include <stdio.h>   //在cpp中，能使用printf还是少使用cout

namespace ghz{

    Thread::Thread()
    :m_thr_id(0)   //线程id初始化为0，这个写法已经习惯了
    ,isRunning(false){
        
        //幸亏单纯对于thread不需要进行初始化，否则高低给他整个RAII
    }

    void Thread::start(){
        
        int ret=pthread_create(m_thr_id,NULL,start_routine,this);
        //一般设计为类的thread类，最后一个珍贵参数都是传this的，作为对于start_routine的static牺牲
        //这一点以后无脑传this即可，一定一定要熟练
    }

    void* Thread::join(){
        void* ret;
        pthread_join(m_thr_id,&ret);
        //这里要养成承接返回值的好习惯，此时就算没有返回值，也可以进行判空操作
        //但是有返回值，就简直跟捡着便宜是一样的

        //如何把承接到的ret返回值返回出来呢？
        return ret;
    }

    //极其严重的错误!!!!
    /*
    和下面的p_this->run()一模一样，都是我原本理解为是多态了，所以才会对这个run进行重写
    但其实这个问题我以前就遇到过——————thread是抽象类基类，并且刚刚在声明文件中写了run是纯虚函数
    然后转头就在这个抽象类中写了run的实现，这实在是太蠢了
    这种只要是抽象类中，基本上正常人就不会前脚设计为纯虚函数，后脚就实现了的这种做法
    所以对于这种做法必须要格外格外注意
    */

    //所以这里的重写是错误的
    // void Thread::run () override {
    //     printf("thread's core code\n");
    //     //写printf的时候一定不能忘了这个换行符，写了这么长时间了确实不应该忘了
    // }

    void* Thread::start_routine(void* arg){
        Thread* p_this=static_cast<Thread*> arg;
        /*
        这里涉及到两点感悟：
        1.对于入口函数，一定一定要切记：入口函数的第一步就是对于参数进行强转
        2.对于类的thread的设计，由于在create时候的this补偿机制，所以一定是无脑转为thread类型的(因为this就是thread类型的)
        3.对于c++使用的是static_cast,一定不要像C语言直接强转了
        */

        p_this->run();
        /*
        之所以不得不使用这个this指针，
        所以这里根本不是在多态，而是单纯是在进行this指针的调用而已
        所以这种写法非常容易被误会为进行多态了
        */

        
        
        //return nullptr;  //入口函数的书写习惯一定要改，改为写exit
        pthread_exit(nullptr);
        //没有返回值也要写，写了才能对join函数展现自己的诚意
    }
}