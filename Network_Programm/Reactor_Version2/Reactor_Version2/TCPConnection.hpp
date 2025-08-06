#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "Socket.hpp"
#include "SocketIO.hpp"
#include "InetAddress.hpp"
/*
TCPConnection类是所有类的枢纽(即可以说所有进行封装的类都是为了让它能顺利工作的)
所以它就像线程池，任务队列这些一样，属于中枢型的类
*/
#include <string>
#include <memory>
#include <functional>    
//原来之前一直报错的"错误类型问题"是由这个库导致的

namespace apion{

class TCPConnection;


using TCPConnectionPtr=std::shared_ptr<TCPConnection>;   
//只有在进行定义的时候，shared_ptr才需要加小括号表明是指向谁的，所以这里也不应该加小括号
using TCPConnectionCallBack=std::function<void(TCPConnectionPtr)>;

class TCPConnection {   
  //这个类表示一个已经建立好的链接，所以是对链接好了之后的各种行为进行的封装
  //所以就是服务器端的所有步骤中，以accept函数为区分点，划分为了两个类

  /*
  又因为进行数据处理和接收发送是一个比较复杂的阶段，所以把SocketIO进行解耦出来，设计为一个类了
  所以SocketIO只和TCPConnection类有关系，原因是：属于是从TCPConnection类中剥离出来的业务逻辑
  这样以后如果需要对IO业务的数据类型进行扩展，只需要在SocketIO类中进行扩展即可，而不需要对TCPConnection类进行任何的修改
  这就是可扩展性和解耦性的最大体现。以后自己设计类的时候，也需要考虑这种多样性，可扩展性(从来没考虑过)
  */

  private:

  /*
  由于在V1版本中，TCPConnection类是架构中的主要汇报人之一，所以所有的类都是组合到它这里，为它所有
  并且这里之所以是组合关系，而不是聚合or关联，是因为TCP链接断开之后，这些内容也就没意义了，所以是有意让TCP链接类管理他们的生命周期的
  所以对于组合关系——————别光想着它占用空间，解耦性不强这些，要想到它在架构中能直接负起管理别人生命周期的作用，这个才是在进行选择组合关系的时候应该做的权衡
  
  与之相关的问题拓展——————如果不是像这样组合的对生命周期进行管理，就要能想到其他管理生命周期的方法——————就比如RAII方法就是除了组合之外，另一种比较好用的管理生命周期的方法
  所以在cpp中，要总能有意识的想到管理生命周期，然后暂时我只能想到这两种实现管理的方法

  所以当某个类是整个架构的地基的时候，其他类都要跑在它之上才有存在意义的时候，设计为组合关系是最最好的关系，这是唯一唯一，组合关系最适合使用的场景
  对于类之间关系的设计，最主要的就是体现在数据成员的设计上了
  */
 Socket m_sock;
 SocketIO m_sockIO;   
 /*
 现在我才想明白——————这个IO类起到了IO线程所需要的一切所需要的努力，所以它是一种极其极其基础的组员，然后非常多的组员都是以它的工作作为封装向上汇报的
 它是依赖地基才能跑起来的，所以它是被组合到地基之上的
 */



//和V1相比，唯三不同的数据成员
TCPConnectionCallBack m_doConnection;
TCPConnectionCallBack m_doInfo;    //用于表达对于传输过来的传输层的信息进行业务处理，所以使用info
TCPConnectionCallBack m_doClose;

  

  public:
  TCPConnection(int fd);
  ~TCPConnection()=default;

  void send(const std::string& msg);
  std::string recv();    //这里明显又是重名函数，所以直接进行使用匿名命名空间即可

  void Shutdown();   //用于关闭整个服务器端的链接
  bool isClosed() const;   //用于判断对端的链接是否关闭了


  //*和V1相比，唯四不同的成员函数
  

    void setAllCallBacks(const TCPConnectionCallBack& cb1,const TCPConnectionCallBack& cb2,const TCPConnectionCallBack& cb3);
    /*
    原本应该是先有这里的setAllCallBacks，然后才有了epoll的setAllCallBacks(即epoll的是为了这里设计的)
    但是又因为epoll要作为这里的参数的传入，然后极其导致受epoll的一对多个tcpconnection的影响，这里无法使用右值引用,
    所以这里就没法优化了，只能对callback设计为const&。所以右值引用并不是callback的标配，而是callback的优化
    */

    void handleNewConnectionCallback();
    void handleMessageCallback();
    void handleCloseCallback();
  

  /*
  所以总结来说，只是优化了设计三个萝卜坑,这三个萝卜坑就是开闭原则的体现，所以这种优化方式仅仅是对于开闭原则的优化，而和epoll没有直接关系
  即：即使没有这个epoll，这几处修改优化依旧是必要的——————所以他们仅仅是优化，不是必要的作为使用epoll才做出的修改
  */


};

}    //namespace apion

#endif //TCPCONNECTION_H
