#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "func.h"

//由于需要把所有的类都视为组件(轮子)进行使用，所以这里就要对于这些轮子的头文件全都包含进来
#include "Acceptor.hpp"
#include "TCPConnection.hpp"
#include "EventLoop.hpp"

#include "ThreadPool.hpp"

#include <string>
#include <functional>



/*
对于V3版本，一定一定要体现出来对于send的没法解耦到IO线程的这种苦恼，这样才能体现出V4版本的巅峰特点(eventfd)所在
所以V3版本是必须要引入线程池的
*/

namespace apion{

/*
using TCPConnectionCallBack=function<void()>;
这里好像没必要写——————原因是这种using都是全局的，只要是引用了头文件，就不需要对全局的内容进行重写
所以同理：这里之所以老师没有对string这些头文件进行包含，就是因为
*/


class TCPServer{
    private:
        Acceptor m_acceptor;
        EventLoop m_evtloop;
        /*
        这就是我前面一直在说的：
        通过组合的极致使用，一些组长，很可能会被别人当作轮子(组员)来使用
        从而就会导致每种版本虽然架构完全相同，但是测试用例的汇报人是完全不同的
        所以测试用例不能决定类的设计方式，而是类的设计方式决定测试用例的写法
        所以现在回看，说测试用例是架构和类的设计之间的桥梁是大错特错的
        架构和类的设计之间的桥梁，只可能是人类对于架构的语言描述
        然后一般来说人(又或者说对于普通程序员)来说，最开始的对于架构的语言描述一定是面向过程的描述
        然后经过后天训练，能对于面向过程所描述出来的内容实现向面向对象的语言描述方式的转化，就是自己在设计思维上的一个极其大的跨越
        我一定要练好这种能力，一定要学会从无到有的类的设计，这个就用reactor进行的类的设计进行练手，这种能力感悟出来了，那将是非常恐怖的面向对象设计师
        */

    public:
        TCPServer(unsigned short port,const std::string& ip="0.0.0.0")
        :m_acceptor(port,ip)
        ,m_evtloop(m_acceptor)  
        {
            /*
            这个构造函数的参数列表比较古怪：尤其是和eventloop的构造函数比...
            eruka! 我明白为什么这么古怪了——————是因为eventloop类中不组合拥有acceptor
            但是这里的tcpserver是组合的关系拥有acceptor
            所以这两者的构造函数的传参才如此的不同
            所以这给了我一个起始：组合关系就要从参数列表就开始回溯，从而负责到底；关联聚合关系只需要传入对应的对象即可，不需要从最开始被组合的类的对象的传参就开始保姆式接管
            难怪我之前一直搞不明白为什么有些需要参数列表中的最原始参数来构造，有些是直接传对象来构造就可以了
            原来是和其两者之间的关系有必然联系——————只有组合才会像保姆一样管的这么宽
            */

        }


        void setAllCallBack(TCPConnectionCallBack&& cb1,TCPConnectionCallBack&& cb2,TCPConnectionCallBack&& cb3){
            m_evtloop.setAllCallBacks(std::move(cb1),std::move(cb2),std::move(cb3));
            /*
            我这里一开始没使用std::move(),这是在太蠢了——————一定一定要对右值和std::move这两者具有绑定的思维
            所以这个函数是直接对标的在eventloop的新链接函数中直接调用conn的setAllCallBack的
            所以在作为进行调用而非对自己的函数进行代码逻辑的书写的时候，就肯定是一行代码结束的，这个是需要建立"调用永远永远都只有一句话"这种思维的
            */
        }

        void start(){
            m_acceptor.ready();
            m_evtloop.loop();
        }

        void stop(){
            m_evtloop.release_loop();   //表示停止循环
        }

};   //end of class TCPServer




//V5就是对于上面这个类的再进一步组合分层————————即把组合这种横向分层的思维用到了极致
//这里实现的是对V3引入的线程池和TCPServer两者之间的封装，目的就是不让线程池作为一个全局变量，而是作为一个类和对象中的元素进行处理
//所以这是一种极致的类和对象思维——————不让任何一个内容掉队，一定要让所有的内容都作为类和对象的一部分


class WORKSever{   //表示用于处理具体业务逻辑的业务逻辑类，所以在横向中，最最右侧(即最上层)的封装了
    private:
    Threadpool m_threadpool;
    TCPServer  m_TCPsev;


    public:
    WORKSever(){

    }

    void start(){     //因为是极致的组合封装了，所以封装到最后就只剩start调研start了，没有任何花里胡哨了
        m_threadpool.start();
        m_TCPsev.start();
    }


    private:   
    /*
    这里写的就是三个萝卜坑的具体实现的代码了
    分析下原本实现是写在哪的？现在写在这是在什么位置进行调用的？
    想起来了，之前是写在测试用例中的，然后在最外层的组合的类中调用的
    然后这里是把函数定义写在这个业务逻辑类中的，然后也是写在最外层调用中的
    不过这里的不同是：原本的实现和调用全都是在main函数中实现的，
    但是这里为了最终的极致的封装，所以设计为了直接把填坑函数放在了成员函数的位置
    所以这里使用的又是用成员函数来初始化构造函数的方法——————这种方法在epoll_crreate来初始化epoll_fd中使用过
    */


    void onConnection(wd::TcpConnectionPtr conn)
    {
        cout << conn->toString() << " has connected successfully.\n";
    } 

    /*
    分析：单纯从携带啊的视角来看，这个萝卜坑的业务逻辑好像从头到尾都没变过
    所以用来写萝卜坑的业务逻辑是永远不需要变的——————还是那句话:IO线程和计算线程是一种逻辑上的概念，而不是直接在物理上体现的概念
    并且另一个问题是：对于IO线程和计算线程的所有业务肯定都是写在这的，因为这里是对于业务逻辑的实现，而不是对于业务逻辑的分离
    就好比这里是提供原材料，但是加工是成员函数需要做的，而不是这个填坑的需要做的
    所以这里的实现代码的内部不管架构怎么变，它都是不变的
    */

    /*
    那么这里用于分离的操作，是那个pendingfunctor，
    然后还要注意由于高度解耦，所以lockguard类需要使用mutex类作为构造函数参数列表来使用
    */
    void onMessage(wd::TcpConnectionPtr conn)
    {
        //onMessage函数是执行在IO线程中的
        //在执行该函数对象的过程中，时间都不宜过长，
        //否则会影响并发的执行

        //read
        string msg = conn->receive();//不能阻塞
        cout << "recv: " << msg << endl;

        //decode
        //compute   //业务逻辑的处理时间不宜过长
        //encode    //否则都会造成并发处理不能保证实时性
        //由于业务逻辑的处理过程是很复杂的，那么就可以将其交给
        //计算线程来执行具体的处理流程
        Mytask task(msg, conn);
        _threadpool.addTask(std::bind(&Mytask::process, task));
        //当计算线程处理完毕之后，再交给IO线程进行发送
        
        //假设执行的回显服务
        string response = msg;

        //send
        conn->send(response);//时间不宜过长
    }

    void onClose(apion::TcpConnectionPtr conn)
    {
        cout << conn->toString() << " has closed." << endl;
        //这种内容totring，我确实没想到——————主要是没想到这个是个什么内容，是不是什么内容都可以toString实现
    }

};

}  //end of namespace apion


#endif //TCPSERVER_H