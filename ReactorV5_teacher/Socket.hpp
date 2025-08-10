#ifndef __Socket_H__
#define __Socket_H__

#include "Noncopyable.hpp"
namespace wd
{

class Socket : Noncopyable
{
public:
    Socket();
    explicit Socket(int);
    ~Socket();

    int fd() const {    return _fd; }
    void shutdownWrite();

private:
    int _fd;
};


}//end of namespace wd


#endif

