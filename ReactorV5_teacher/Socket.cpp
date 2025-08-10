#include "Socket.hpp"
#include <func.h>

namespace wd
{

Socket::Socket()
{
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if(_fd < 0) {
        perror("socket");
    }
}

Socket::Socket(int fd)
: _fd(fd)
{}

Socket::~Socket()
{
    close(_fd);
}

void Socket::shutdownWrite()
{
    //主动断开连接时调用，只关闭写端
    shutdown(_fd, SHUT_WR);
}

}//end of namespace wd
