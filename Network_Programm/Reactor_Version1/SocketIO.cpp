#include "SocketIO.hpp"


namespace apion{

    int SocketIO::readn(char* buf,int len){

    }   
 
  int SocketIO::sendn(const char* buf,int len){

  }

  int SocketIO::readline(char* buf,int maxlen){

  }

  int SocketIO::recvpeek(char* buf,int maxlen) const{
    //这里再次提醒我了：对于static在定义的时候不用加，但是const在定义的时候必须要加。这两者一定要区分清楚了

  }
    
}
