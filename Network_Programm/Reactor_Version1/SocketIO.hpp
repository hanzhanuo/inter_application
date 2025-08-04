#ifndef SOCKETIO_H
#define SOCKETIO_H

namespace apion{

class SocketIO {

  private:

  int m_fd;
  

  public:
  SocketIO(int fd)  //用于表示对哪个fd的内容进行IO操作
  :m_fd(fd){
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