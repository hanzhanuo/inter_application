#include <iostream>
#include <pthread.h>   
//只要是有.h的，就一定是C语言的库。并且这里的目的就是通过C语言的库对cpp线程库进行造轮子
#include "noncopyable.hpp"


using namespace std;


namespace thread{  //对于命名空间内部不需要进行空行，否则就会导致和其他代码的格式每次都差一行


class Thread:public noncopyable::Noncopyable 
/*
这里对于继承我有很多想要说的：
1.继承使用的是单冒号，不要使用双冒号
2.这里由于我写在命名空间中了，所以既是在继承的时候也需要带命名空间。这个非常重要，我每次都会忘记这个
(续2)只要是涉及到这个类的，前面都必须加类名作用域限定符。一定要记住这种"资源使用方法"
*/


{

    private:
    pthread_t m_thr_id;
    bool m_isRunning;  
    //对于线程问题，这个标志位是必须有的，连编译器都知道这个特点了

    


    public:

    void start();
    void join();
    //这两个函数从代码逻辑结构上交相辉映，
    那么从功能上呢？是否交相辉映？毕竟设计类的时候是从功能入手进行思路分析的，而不是从代码结构上入手进行思路分析的
    但是话又说回来了，如果从结构上都交相辉映了，那么功能上可能不相同吗？

    void detach();   //这个函数究竟是用来干嘛的？

    Thread() 
    : m_isRunning(false) 
    , m_thr_id(0) {

    }  //构造函数中初始化标志位

    virtual ~Thread() {
        if (m_isRunning) {
            pthread_detach(m_thr_id);  //如果线程还在运行，则分离线程
        }
    }  //析构函数中需要进行线程的分离操作，防止内存泄漏

    //虽然这里写的很好，并且告诉了我一个哩哩老师以前讲过的：对于虚函数原来是可以直接实现的


    //我始终认为：这种简单的函数直接在头文件中实现即可(从效果上来看就和lambda表达式一样,专门用于写篇幅小的)
    //只有篇幅较大的函数才有必要在实现文件中写
    
    virtual void run() = 0; 
    //这个函数作为纯虚函数使用，必须是所有函数中第一个被想到的
    

    static void* start_routine(void*);

};
}

// int main(){}   //头文件中是不需要有main函数的，所以我又一次写的没有头绪乱写了

