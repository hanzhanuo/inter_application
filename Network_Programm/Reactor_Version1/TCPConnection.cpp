#include "TCPConnection.hpp"



namespace apion{


  
  void TCPConnection::send(const std::string& msg){

  }

  std::string TCPConnection::recv(){

  }  

  void TCPConnection::Shutdown(){

  }  
  
  
  bool TCPConnection::isClosed() const{
    //进行close判断，一定是使用peek的，如果是传0,就会直接把数据转移了，而不是查看了
  }


}