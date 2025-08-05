#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "Socket.hpp"
#include "SocketIO.hpp"
#include "InetAddress.hpp"
/*
TCPConnection类是所有类的枢纽(即可以说所有进行封装的类都是为了让它能顺利工作的)
所以它就像线程池，任务队列这些一样，属于中枢型的类————————它的中枢性质是由架构中的事件处理器和事件分离器两个核心组件决定的


我现在还发现了：根据类设计的不同，测试用例的写法也是不同的——————原因是类的设计不同就可以理解为选举出了新的小组长
所以上去汇报的小组长不同了，测试用例当然也就不同了
最典型的就是在V1的时候，TCPConnection类还是小组展示的骨干展示成员，但是现在写到V3，它就已经被调整结构下去了(即不作为进行汇报的组长了)
所以我的组员组长的思维模型，还可以配合V3和V5，进一步提炼为：
在不同的舞台上(不断组合所产生的"横向抽象分层"，就是不同层级的舞台),
进行汇报的组长也不同(因为组长之间的层级亦有不同，在下面层级可以当组长的，可能越到上面层级就只能当组员了)
这就是V3V5两个版本的核心优化给我带来的感悟
*/
#include <string>
#include <memory>

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

  //这个还没写完

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
