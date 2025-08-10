#ifndef __InetAddress_H__
#define __InetAddress_H__

#include "func.h"

#include <string>
using std::string;

namespace group_6
{

class InetAddress
{
public:
    InetAddress(unsigned short port, const string & ip = "0.0.0.0");
    InetAddress(const struct sockaddr_in &);

    string ip() const;
    unsigned short port() const;

    struct sockaddr_in * getInetAddressPtr() {  return &_addr;}

private:
    struct sockaddr_in _addr;
};

}//end of namespace group_6


#endif

