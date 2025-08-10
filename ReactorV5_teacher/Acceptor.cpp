#include "Acceptor.hpp"
#include <asm-generic/socket.h>
#include <sys/socket.h>


namespace wd
{

Acceptor::Acceptor(unsigned short port, const string & ip)
: _addr(port, ip)
, _sock()
{}


void Acceptor::ready()
{
    setReuseAddr(true);
    setReusePort(true);
    bind();
    listen();
}

int Acceptor::accept()
{
    int netfd = ::accept(_sock.fd(), nullptr, nullptr);
    if(netfd < 0) {
        perror("accept");
    }
    return netfd;
}

//设置网络地址可以重用
void Acceptor::setReuseAddr(bool on)
{
    int value = on;
    int ret = setsockopt(_sock.fd(),
                         SOL_SOCKET,
                         SO_REUSEADDR,
                         &value,
                         sizeof(value));
    if(ret < 0) {
        perror("setsockopt");
    }
}

//设置端口可以重用
void Acceptor::setReusePort(bool on)
{
    int value = on;
    int ret = setsockopt(_sock.fd(),
                         SOL_SOCKET,
                         SO_REUSEPORT,
                         &value,
                         sizeof(value));
    if(ret < 0) {
        perror("setsockopt");
    }
}

void Acceptor::bind()
{
    //使用匿名命名空间访问Linux下的bind函数
    int ret = ::bind(_sock.fd(),
                   (const struct sockaddr*)_addr.getInetAddressPtr(),
                   sizeof(_addr));
    if(ret < 0) {
        perror("bind");
    }

}
void Acceptor::listen()
{
    int ret = ::listen(_sock.fd(), 10000);
    if(ret < 0) {
        perror("listen");
    }
}

}//end of namespace wd
