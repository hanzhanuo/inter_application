#include "Thread.hpp"
//#include <log4cpp/Log4cpp.hh>

using namespace std;

namespace thread {   
    //果然当在头文件中写了命名空间的话，实现文件中也必须写命名空间才能进行实现


    //一定是先整体后局部的写法，才导致先想到了写入口函数，然后写着写着入口函数才想起来写run方法
    //所以这种倒序方法的书写才是最优秀的写代码思路
    void Thread::run()override{
        cout<<"run"<<endl;
        //这里是对于run方法的实现，入口函数就是对于run方法的封装
        //之所以这样进一步封装，是因为没法对纯虚的run方法直接调用，必须要通过多态调用
        //由于没法直接调用所以就不能像入口函数一样作为函数对象直接传入，而是必须多态指针调用

        //所以这就是run方法和入口函数之间的分工操作
    }

    void* Thread::start_routine(void* arg){   //子线程的入口函数
        
        //对于进行封装成类的线程库，子线程的入口函数要做的内容需要和原本linux中子线程需要做的做对比

        //比如最最典型的就是这里要对多线程中的标志位进行修改

        m_isRunning = true;
        Thread* p_thread=static_cast<Thread*>(arg);
        //这里的进来再转换，要和数据库抽象类的直接传入基类指针做对比
        //并且还需要和直接对于void*的形参里直接传入其他类型的指针做对比
        //这三者现在看来都是合法的
        //虽然三种都合法，但是三种方法的常用使用场景各不相同。所以要总结他们的使用场景的区别
        
        //ai给的一些使用场景的拓展————————这三个传入什么的归纳，指的是create函数中传入的是什么
        //就像下面说的，因为这里以牺牲了this指针作为代价，所以传入this指针是最好的，最常用的传参方法
        //1.直接传入this指针，这个是最常用的，也是最方便的，但是只能在多态的情况下使用
        //2.直接传入其他类型的指针，这个是最不常用的，也是最危险的，因为其他类型的指针可能会指向其他类型的对象，所以要非常小心
        //3.直接传入void*指针，这个是最不常用的，也是最危险的，因为void*指针可以指向任何类型的对象，所以要非常小心

        p_thread->run();
        
        return nullptr;   //因为是void*的指针类型作为返回值，所以默认返回空指针是最最好用的方法

    }

    



    void Thread::start() {

        if (m_isRunning) {   //我认为我这种判断写的比老师好，更多了一层健壮性，老师写的是只有不running才写代码逻辑
            cout << "Thread is already running." << endl;
            return;  //如果线程已经在运行，则直接返回
        }
        int ret=0;
        ret=pthread_create(&m_thr_id, nullptr, start_routine, this);

        //重点就是这个传this，并且这个传this是受入口函数由于参数列表的个数和顺序限制，所以必须设计为静态函数，从而没有this指针来调用run方法所导致的
        //之前还学过的一个由于this指针的顺序问题所以不得不设置为外部友元的，就是比较运损付的重载？


        /*
        关于这里的返回this指针的这个技巧的评价：
        1.因为这个本来就是面向对象设计，所以必须要设计成成员函数，然后就导致了：只要有严格的参数列表就不能设计为普通成员函数，最好就是设计为静态函数
        2.这里传入this指针的思维理论是那两套循环系统作为基础，所以才能想到传入this指针进行操作的
        3.create函数的参数中原本就是要求传入一个void* 一级指针，根据void*的兼容性，所以这里才可以直接传入this指针
        4.前面是对于传入this指针的理论可行性，这里是针对传入this指针的必要性来分析：
        (接4)this指针之所以必须要在这个函数中使用，是因为这个函数要调用多态的run方法。只要是多态方法，就必须用指针调用，这里就是必须使用this指针调用吗？
        */




        //这个函数绝对不只是我上面写的这么简单——————在前面linux的pthread's_use里面已经写出来了：
        //即对于主线程的create函数和join函数都需要进行错误判断，所以这些错误判断理所应当直接封装到这个start函数中
        //所以如果想要造轮子，就要对初始使用的时候都有什么大致步骤，就全都要封装进来，这才是造轮子的最佳设计处理思路
        if(ret!=0){
            cout<<"pthread_create error"<<endl;
            exit(1);  //exit(1)表示异常退出
        }
    }   //end of func start() 


    void Thread::join(){
        if (m_isRunning) {
            int ret=0;
            ret=pthread_join(m_thr_id, nullptr);
            if(ret!=0){
                cout<<"pthread_join error"<<endl;
                exit(1);  
            }
            m_isRunning = false;  //设置线程为未运行状态
        }
        //这里的join函数和c++11线程库中的join函数的区别在于：c++11线程库中的join函数是直接调用线程对象的join方法，而这里则是通过pthread_join函数来实现的
        //所以这里的join函数和c++11线程库中的join函数是等价的    
        
               
    }   //end of func join() 




    


}