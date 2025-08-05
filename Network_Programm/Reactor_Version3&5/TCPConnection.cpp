#include "TCPConnection.hpp"



namespace apion{


  void TCPConnection::send(const std::string& msg){

  }

  std::string TCPConnection::recv(){

  }  

  void TCPConnection::Shutdown(){

  }  
  //bool TCPConnection::isClosed() const{}  //在声明后定义的时候，不能加这个const
  
  bool TCPConnection::isClosed() const{
    //进行close判断，一定是使用peek的，如果是传0,就会直接把数据转移了，而不是查看了
  }


  void TCPConnection::setAllCallBacks(const TCPConnectionCallBack& cb1,const TCPConnectionCallBack& cb2,const TCPConnectionCallBack& cb3){
    
    //这里由于是TCPConnection的一对多的关系，所以就没法使用移动语义来实现了，只能使用拷贝语义
    //所以这里才不使用std::move()，而是直接使用赋值运算符来实现
    m_doConnection=cb1;
    m_doInfo=cb2;
    m_doClose=cb3;
  }
    

    void TCPConnection::handleNewConnectionCallback(){

    }
    void TCPConnection::handleMessageCallback(){

    }
    void TCPConnection::handleCloseCallback(){
      
    }
  


}
