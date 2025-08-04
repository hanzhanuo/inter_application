#include "Socket.hpp"

namespace apion
{

/*
这里只需要切记——————是在使用RAII思想来管理的socket的
只要能记住这个，就能记住这个函数内部应该怎么写
*/
Socket::Socket(){
    m_fd=socket(AF_INET,SOCK_STREAM,0);
    if(m_fd<0){
        perror("socket");
    }
}
Socket::Socket(int fd)
  :m_fd(fd)
{
    //这里由于不需要进行socket来创建fd了，而是手动配置的。所以自然不需要进行socket函数实现了
}
Socket::~Socket(){
    close(m_fd);
}

void Socket::shutdown_write(){
    shutdown(m_fd,SHUT_WR);
}

}