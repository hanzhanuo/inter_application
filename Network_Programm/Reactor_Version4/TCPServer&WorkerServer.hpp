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


using std::string;    
/*
注意所有的using(无论是用来重命名的还是用来声明std的)
都是无脑写在namespace紧跟着的位置的
这里我写这个std::string,是因为我发现这个我用到的极其频繁，所以只要是用到的频繁的std内容，就一定要using，不要一个一个std
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



class MyTasks{       
    /*
    这里的mytasks就是线程池的mytasks，这里写的process就是执行的业务主逻辑，所以此时原本用来执行业务主逻辑的onMessage函数就用来执行线程池了
    即mytask抢了onMessage，但是由于onMessage是最终执行的最大的部分，所以这里process把他的工作分走了，他当然要执行更宏大的操作了————————即执行启动线程池的操作
    */

    /*
    并且这里执行的是最终的效果是：process把计算线程所做的任务给封装起来了，但是在调用的时候还是在onMssage函数中调用
    所以就是onMessage最终从结果上看，还是执行了五步(因为执行5步是必须的，这里的思想仅仅是对于计算和IO进行解耦，而并不是两者不能同时被调用了)
    而且一定要切记：这里的计算线程被解耦就是这里的process这个萝卜坑中做的事情，所以这个确实和线程池一样，属于是各种各样的业务逻辑，
    ——————所以也可以看出来：线程池这种东西天生是用来处理具体业务逻辑的，而不是用来处理网络IO的(即这里的IO指的是网络IO，而不是本地IO)
    因此以后只要有设计到计算线程(即处理IO以外的各种五彩缤纷的业务逻辑)，就毫不犹豫选择BO线程池即可
    ——————BO的线程池方法是开闭原则最强的，比之前推崇的设计为onMessage更强(因为这里就是对onMessage的萝卜坑思想进行一步更极致的优化)
    

    注意点1：解耦并不是不调用，而是分开封装，然后对每个封装进行调用。所以原本在最终位置需要调用什么函数，在修改升级后，需要调用的一个也不能少
            只不过调用的形式变了，从而很容易看起来没有调用，但其实一步步调试会发现都封装起来了

    注意点2：IO线程是指的网络IO，即read(收)和send(发)。但是由于这两个操作中间被三步挡住了，
            所以这两步是不可能封装起来的，只能实现不在那三步的封装里执行————————这种中间有内容的解耦，最多就只能解耦到这个地步了
            ———————即没法把这些步封装起来，就把除了它们以外的所有步都封装起来，这样就能实现对于他们反向视为封装了

    */
    

    private:
    
    TCPConnectionPtr m_conn;   //具体是对于那个TCP链接？这条是mytask必须要有的
    const std::string& m_msg;  
    /*
    这个和业务逻辑直接相关，这回发过去的是一个string，下回其他业务场景的业务逻辑发过去的就可能是一个int
    所以可以说整个Reactor模型中，只有这里的业务逻辑是最多变的，其他代码全都是照搬即可，没什么需要根据业务场景进行大修大改的位置
    所以这里的process这个业务逻辑函数:一般都是需要传入传出参数的
    */
    
    


    public:

    MyTasks(TCPConnectionPtr conn,const std::string& msg)
    :m_conn(conn)
    ,m_msg(msg)
    {
        /*
        这里限制了TCP作为MyTasks的数据成员，但是我认为这不是束缚，相反这是一种"乘法效果"
        ——————即原本只能实现某个链接中实现不同的业务逻辑，现在就可以实现每个链接都可以实现许多不同的业务逻辑——————所以我才说这是乘法效果
        所以这里很容易混淆一个概念：只要是这个类中的这些process，那就是不管是被什么TCP链接限制住的对象，都是可以使用这些所有的process成员函数的
        是可选择并且全部都可用的——————千万不要想成是TCP链接限制住了只可以使用哪些process了
        */
    }

    void process1(){   //作为回调函数，一定要设计为返回值是void的，因为bind只能修改参数，不能修改返回值
        //decode 
        //comupt
        //encode

        std::string response=m_msg+"1";   //假设这个就是经过上面的业务逻辑所得到的结果
        
        m_conn->send(response);     //我故意把send函数写在计算线程里了
        /*
        因为这里是V3,所以我刻意写成了这种：把send没有解耦出去，依旧写在这个计算线程中的IO行为
        V3为什么没有解耦出去？原因是计算线程的三步卡在了IO线程的两步中间，找顺序执行的代码逻辑，肯定是没法把第一第五步封装起来的
        但是后面V4引入了eventfd这种线程间通信的机制，从而使得把原本的直接send，通过操作强行扭成了先本地的线程间通信，然后再让IO线程专门用于进行网络IO操作
        所以这里就是强行拧巴为了：把原本一个本地线程就能实现的内容，拆分托管给了两个本地线程才能执行的过程
        而这种拧巴不会带来性能上的提升(相反由于线程间通信，所以性能下降了)，但是让程序员后续维护代码的时候更好维护了
        所以可以看出来：可维护性远远远大于一点点的性能损耗
        

        并且这里还需要注意一个点：IO线程和计算线程只不过是人脑中的逻辑结构，而不是在代码中实实在在体现的
        其体现的是：把网络流程中的通识性的必要流程抽象出来了两大类，这两大类步骤被归纳为了：进行业务计算的步骤，还有进行网络IO的步骤
        所以这里的执着于解耦，就是因为它并不是代码体现的所谓的线程，而是人脑抽象出的所有步骤按功能划分可以分为几大类
        */
    }


    void process2(){

        //decode
        //compute
        //encode


        std::string response=m_msg+"2";

        m_conn->send(response);
        //我有一次写为直接send了，一定一定要小心这里，非常容易写错

    }


};

//V5的最终产物(也可以说是V5的唯一产物)
//所以难怪我一开始比较费解为什么和老师教案中V4的onMseeage写法不一样，原来是因为这里其实是V5的内容了
//所以这里对照着老师的教案看的时候，一定要小心，这里对照着看很容易疑惑
class WORKSever{   //表示用于处理具体业务逻辑的业务逻辑类，所以在横向中，最最右侧(即最上层)的封装了
    private:
    Threadpool m_threadpool;
    TCPServer  m_TCPsev;


    public:
    WORKSever(int threadNum,int queSize,unsigned short port,const std::string& ip="0.0.0.0")
    :m_threadpool(threadNum,queSize)
    ,m_TCPsev(port,ip){    //我这里犯的错误实在太蠢了——————我把

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


    void onConnection(TCPConnectionPtr conn)
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
    void onMessage(TCPConnectionPtr conn)
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
        MyTasks task(conn,msg);
        _threadpool.addTask(std::bind(&MyTasks::process1, task));
        //当计算线程处理完毕之后，再交给IO线程进行发送
        

        /*
        在V4版本中，总算把对最后一步send的操作进行分离的问题给解决了
        所以这里体现的是：所谓的对IO线程解耦，其实就是对第一步和最后一步进行反向的丢弃
        所以这种反向丢弃的操作，就是对于不是顺序执行的步骤的最优解耦方式
        */

        //假设执行的回显服务
        string response = msg;
        //send
        conn->send(response);//时间不宜过长

        
        
    }

    void onClose(TCPConnectionPtr conn)
    {
        cout << conn->toString() << " has closed." << endl;
        //tostring函数是自定义函数，所以我之前一直以为是string的库函数了，我实在太蠢了
    }

};

}  //end of namespace apion


#endif //TCPSERVER_H