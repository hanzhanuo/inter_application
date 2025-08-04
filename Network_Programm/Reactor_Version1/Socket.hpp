#ifndef SOCKET_H
#define SOCKET_H

#include "Noncopyable.hpp"
#include "func.h"

namespace apion{

class Socket :public Noncopyable{

  private:
    int m_fd;

  public:

  //成员函数声明顺序(几大基本函数，各种功能性函数，最后才是放各种getter函数)

  Socket();
  Socket(int);   
  /*
  这是为了实现对于所有的fd都能实现初始化
  (由于后面需要手动配置fd的数字，所以这里才需要有一个这样的有参构造函数的)
  */

  ~Socket();

  void shutdown_write();    
  /*
  用于封装全双工TCP通信时的四次挥手的问题，
  一般来说四次挥手只可能是主动关闭写端，不可能是主动关闭读端
  这样记忆就能记清楚什么时候使用这个函数了
  */


  //getter永远放在最后面
  int get_fd() const{      
    /*
    所有的getter函数都应该写为const类型的
    就像基本上所有的参数列表中的string都需要写为const类型一样
    */
    return m_fd;
  }
    
};

}

#endif //SOCKET_H