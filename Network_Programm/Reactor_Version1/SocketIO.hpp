#ifndef SOCKETIO_H
#define SOCKETIO_H

namespace apion{

  /*
  SocketIO类自打V1之后就再没变过，所以这就是解耦的最高标准
  这种解耦就是在设计的时候就设计的非常成功了，所以在我自己想的时候，也要能想到对于计算线程和IO线程要分离
  但是这里并不是线程分离(因为线程分离应该体现在函数调用上而不是函数定义上)
  所以这里做的是职责分离，而不是架构中的线程分离。
  */

class SocketIO {

  private:

  int m_fd;
  

  public:
  SocketIO(int fd)  //用于表示对哪个fd的内容进行IO操作
  :m_fd(fd)
  {
    //以后写构造函数的规范就要是把花括号换行，然后独立于初始化列表存在
  }   
  ~SocketIO()=default;

  int readn(char* buf,int len);   
  //注意一定是char*类型，因为网络编程就都是cahr*类型的
  int sendn(const char* buf,int len);  //只要是getter和send，这两种操作就都是cosnt不需要修改内容的操作

  int readline(char* buf,int maxlen);

  int recvpeek(char* buf,int maxlen) const;
  //因为这个操作要反复使用，所以这里具把它封装为一个函数了

};

}

#endif //SOCKETIO_H