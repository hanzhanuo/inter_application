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

namespace apion{

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

  public:
  TCPConnection(int fd);
  ~TCPConnection()=default;

  void send(const std::string& msg);
  std::string recv();    //这里明显又是重名函数，所以直接进行使用匿名命名空间即可

  void Shutdown();   //用于关闭整个服务器端的链接
  bool isClosed() const;   //用于判断对端的链接是否关闭了


};

}    //namespace apion

#endif //TCPCONNECTION_H
