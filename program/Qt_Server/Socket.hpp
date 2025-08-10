#ifndef __Socket_H__
#define __Socket_H__

#include "Noncopyable.hpp"

namespace group_6
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


}//end of namespace group_6


#endif

