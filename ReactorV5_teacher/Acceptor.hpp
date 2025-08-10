#ifndef __Acceptor_H__
#define __Acceptor_H__

#include "InetAddress.hpp"
#include "Socket.hpp"

namespace wd
{


class Acceptor
{
public:
    Acceptor(unsigned short port, const string & ip = "0.0.0.0");
    void ready();
    int accept();
    int fd() const {    return _sock.fd();  }
private:
    void setReuseAddr(bool);
    void setReusePort(bool);
    void bind();
    void listen();

private:
    InetAddress _addr;
    Socket _sock;//不可复制的，所以Acceptor对象也不能复制
};

}//end of namespace wd


#endif

