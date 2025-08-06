#ifndef __EventLoop_H__
#define __EventLoop_H__


#include "func.h"
#include "Mutex.hpp"
#include "TCPConnection.hpp"
#include <vector>
#include <unordered_map>
#include <memory>   //智能指针
#include <functional>   //基本上都是回调函数才使用这个库进行砸坑操作
#include <sys/epoll.h>   //切记只有这个eventloop类需要这个库函数，所以它才没被严选到func.h中

namespace apion{

//切记所有的using都是在头文件的类和命名空间中间的位置进行定义的，一定要熟练




class Acceptor;    //只写前向声明，不需要写头文件
/*
只要是不使用行为，并且使用的对象还是指针类型的，就可以只使用前向声明不使用头文件
所以单来看这个两个要求，还是挺苛刻的
TCPConnection类需要加头文件的原因是下面的shared_ptr的定义就需要它的大小了
所以所有的类都需要前向声明，但是不是所有的类都不需要头文件的;并且有了头文件其实就没必要写前向声明了，所以这里写TCPConnection的前向声明纯属没意义
*/

//在TCPConnection内部定义using的时候，由于自己不认识自己，所以必须要加前向声明
//而这里由于只需要头文件就能认识TCPConnection，所以就不需要加前向声明了
using TCPConnectionPtr=std::shared_ptr<TCPConnection>;
/*
对于只能指针的类型的写法，就是直接写尖括号。感觉都已经忘了，一定一定要熟练复习
这里之所以创建指针，是因为TCPConnection不能被复制，所以使用指针就实现了地址传递，就避免了进行拷贝的操作
然后这里确实是使用map来管理tcp链接，因为只需要最最关键的fd就能实现查询和管理，所以自然是不需要使用vector结构的
*/

using TCPConnectionmap=std::unordered_map<int,TCPConnectionPtr>;
/*
这种设计太NB了，就应该多设计这种对于很长的串的重命名，
所以using仅仅是用来改名的而已，function才是用来砸坑的
因此以后不要只要看见了using就是进行回调函数，using就是最最纯粹的对于长内容进行重新取名的
所以为了保证准确，就进行尽可能对应的取名——————就比如这里的map就取名map，function就取名回调函数
*/

/*
但是老师没对这个map进行using重命名，是因为重命名只适合是用在需要频繁使用的长内容上，
这里不是需要频繁使用的，通篇只需要使用一次即可,所以自然没必要进行using了
所以这个和上面的总共两个注释，把using的使用场景说的非常非常透彻了
*/

using TCPConnectionCallBack=std::function<void(TCPConnectionPtr)>;
/*
切记这里只要是回调函数，就一定是void()的形式，因为回调函数的返回值是固定的，就是void
这个点我倒是从来没发现过，谢谢AI帮助我洞察
使用function基本上可以直接定性了——————一定是使用的是回调函数
*/


using Functor=std::function<void()>;   //专门给eventfd的相关绑定所使用的using


class EventLoop{

  private:
      int m_epoll_fd;
      Acceptor& m_acceptor;
      /*
      这里之所以必须和Acceptor进行关联，是因为acceptor中实现了listen和accept两种操作的封装
      而epoll中必须要对listen_fd进行监听，还必须要对accept的新net_fd进行监听添加
      所以想要实现这两种操作，就不得不去找acceptor类才能实现，其他的都无法实现
      所以封装就是划分领地范围，而有求于人则是必须要上别人的领地上去请求帮助。所以自然要设计为关联or聚合了
      */

      bool m_is_looping;  
      /*
      这个是用于避免代码之间的解耦封装所导致的没法停止原本的while(1)循环所设计而得的标志位
      所以这里给了我启示:如果以后有些代码在解耦的过程中导致有些步骤变成了三不管的逻辑地带，就可以通过设计标志位的方式实现对于这个三不管地带的接管
      因为标志位本身一定是全局的，所以一定是会被设计成bool类型数据成员的。
      通过对这个全局变量的共同修改，就能实现三不管地带的上下文代码配合，标志位来实现承上启下的作用
      */

      //std::vector<struct epoll_event> m_evtArr={0,EPOLLIN}; 
      std::vector<struct epoll_event> m_evtArr;  
      /*
      用于拷贝epoll的就绪队列
      第一遍当作理解我可以接受，但是后面如果再写一遍还是没法默写下来这些内容，我就太fw了
      这个结构体显然是在epoll中原封不动搬过来的，所以这个记不住实在不应该
      */

      /*
      我上面注释中的写法也可以，老师的直接在构造函数的初始化列表中传入一个数字1000了，
      我也不知道这两种传入的区别是是什么，所以主要还是我对于vector的各种初始化方法实在太生疏了
      这种作为STL的最最最基础的基本功我都能忘，实在是耻辱——————这根本就不是一个合格的程序员

      我认为老师的初始化方法是设置这个evtArr的大小——————因为这个数组是用来承接epoll的输出的，而不是手动初始化之后传入epoll中作为输入使用的
      所以这个vector只需要关注大小(因为大小是到时候epoll_wait接口的必要参数)，而不需要关注其内部元素初始化为什么值
      所以这里可以看出：对于这些STL进行初始化，关注点应该在需求，由这个vector的需求，来决定使用什么方式进行初始化
      就比如这里的需求是只需要设计vector的大小即可，其他细节都不需要，那么就在初始化列表中无脑使用(1000)进行初始化即可
      */

      std::unordered_map<int,TCPConnectionPtr> m_connMap;
      /*
      用于管理tcp连接——————因为epoll相比于V1版本，是一对多的结构，而不是还有一个TCP链接了，所以就必须要设计一个数据结构对这个TCP链接进行管理
      这里之所以选择map，还是因为受到了epoll_ctl的键值对的设计管理方式的启发
      并且之所以不使用vector，而是使用同样乱序的unordered_map，是因为TCPConnectionPtr受socket影响，是不可复制的。
      而vector的底层实现原理是通过复制来实现的，所以就不能使用vector组织，而是使用等价于vector的下标访问的key为int类型的map来实现组织
      所以看似使用了比vector更高级的unordered_map，但是其实本质上这里的unordered_map就是使用的套壳的vector
      所以这种如果不能使用vector，就是用套壳的key为int类型的map的方法，一定一定要产生经验
      */


      //设计三个萝卜坑，到时候实现开闭原则的对于代码进行扩展，最最关键的就是这三个用于cin的萝卜坑
      TCPConnectionCallBack m_doConnection;
      TCPConnectionCallBack m_doInfo;    //用于表达对于传输过来的传输层的信息进行业务处理，所以使用info
      TCPConnectionCallBack m_doClose;


      //被封装的成员函数统统设计为private的，所以下面这些都是被封装的函数
      int epoll_create();
      void epoll_addReadEvent(int fd);   //用于填充ctl函数的第三个参数————即对谁进行添加
      void epoll_delReadEvent(int fd);
      void epoll_fdwait();
      //上面这四个函数就是epoll的三个接口，epoll的最核心价值就是这撒个接口

      void handle_NewConn();    
      /*
      这个函数可以说是这个epoll封装中数一数二重要的函数(原因是它的改动是最大的，其他的函数的代码基本上都没怎么改动)
      这个函数是用来实现萝卜坑的传递的
      */
      void handle_OldConn(int fd);   
      //用于在就绪队列数组中查找是哪个已有链接好了，所以这两个函数是用来分别对应epoll的代码中对于新链接和旧链接的处理的


  public:
      EventLoop(Acceptor& acceptor);   
      /*
      能写引用就不要写指针
      这里写一百遍都可能记不住:对于构造函数的参数，纯纯是用来实现初始化列表的，所以不要把它和成员函数的参数列表搞混了
      成员函数的参数列表中如果是另一个类的成员的话，一般就意味着是依赖关系;
      但是构造函数的初始化列表中如果是类的话，就只可能是其他四种关系，不可能是依赖关系
      原因是构造函数的参数一定是用来初始化数据成员的，
      所以如果另一个类的成员都出现在一个类的数据成员中了，那么当然必然是其他四种关系，不可能是依赖关系了
      */
      ~EventLoop();

      void setAllCallBacks(TCPConnectionCallBack&& cb1,TCPConnectionCallBack&& cb2,TCPConnectionCallBack&& cb3);
      /*
      以前我一直理解为这里写右值引用仅仅是专门为bind服务的，这里让我拓宽了眼界——————函数名也是右值，也可以作为参数传入右值引用中
      并且由于右值引用的移动语义极其优秀特性，所以以后尽可能(注意是尽可能不是强行)设计为右值引用
      比如这里的尽可能就体现在函数名就是右值，所以就可以直接把函数名传入来作为右值引用来

      */

      void loop();    
      /*
      loop函数中封装了epoll内部所需要的循环的所有内容，所以它是属于用于封装其他行为的函数
      只要是被封装的，不是直接调用的函数，就都是最好设计为private的函数，这就是其中一条类和对象的设计思维
      因为最开始在学习cpp的定义的时候，就学了对于private的内容，无法在类外调用(即无法通过对象实例直接调用)，所以竟然连这个都忘了
      */

      void release_loop();   //和m_is_looping标志位配合，才能实现解除循环的操作


      /*
      对于所有饿V4版本的eventfd中的内容，除了需要在TCPConnection中写send_in函数来调用run_in函数之外.
      其他的所有内容都是放在eventloop中的
      原因是这里进行的是对于send函数的传输(所有所有函数都是可以bind的，而不是只有那三个on填坑函数才可以实现传入)
      */
      

      


      

      //eventfd的模块：
      public:
      void run_inLoop(Functor&& cb);   //所有右值引用传的一定是cb，即右值引用一定是和回调函数始终在一起使用的
      private:
      /*
      我总算明白为什么一些非常复杂的类需要反复写多个public和private了
      原因就是这样每一个模块的内容放入了这个类中，就可以集中把这个模块的所有内容放在一个位置了
      */

      int m_eventfd;   //所以这里的这些相关模块和数据成员全部都放在一起，这样还可以方便助记，能立刻想起来需要写什么
      vector<Functor> m_PendingFunctor;
      Mutex m_mutex;

      //分析每个函数的含义以及其两两对应的关系
      int createEventfd();   //只要是fd的create函数，就必定是返回值为int，这个每次写诶次都错，真是不应该错...
      //所以以后每次看到create函数，就一定是返回值为int

      void handleReadEvent();   //进行read操作
      void do_wakeup();   //write操作，通过写操作使得内核计数器+1
      void do_PendingFunctors();  //对send的数组进行执行的操作
      //run_in函数是对这个send数组进行插入操作

      /*
      所以现在总结出来了：这里的取名取得不好，五个函数中除了创建fd的函数外，剩下四个函数两两对应
      所以应该在取名的时候取为两两对应的名字,这样助记，这四个函数就能一下记下来写在一起了
      */



      //end of eventfd模块

};

}  //namespace apion


#endif  //end of __EventLoop_H__ 