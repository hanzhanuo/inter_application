#ifndef __TcpConnection_H__
#define __TcpConnection_H__
#include "Socket.hpp"
#include "SocketIO.hpp"
#include "InetAddress.hpp"

#include <memory>
#include <functional>
using std::shared_ptr;
using std::function;

namespace wd
{

class EventLoop;

class TcpConnection;
using TcpConnectionPtr=shared_ptr<TcpConnection>;
using TcpConnecitonCallback=function<void(TcpConnectionPtr)>;

class TcpConnection
//辅助类，为了在成员函数内部通过this直接获取本对象的智能指针shared_ptr
: public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(int fd, EventLoop* loop);
    ~TcpConnection();

    //TcpConnection内部的setAllCallbacks函数不能采用移动语义
    //必须要表达值语义, 即不能把EventLoop对象的三个函数对象
    //直接转移到TcpConnection对象中, 因为TcpConenction对象不止一个
    void setAllCallbacks(const TcpConnecitonCallback & cb1, 
                         const TcpConnecitonCallback & cb2,
                         const TcpConnecitonCallback & cb3)
    {
        _onConnection = cb1;
        _onMessage = cb2;
        _onClose = cb3;
    }

    string receive();
    void send(const string & msg);
    void sendInLoop(const string & msg);

    bool isClosed() const;
    void shutdown();
    string toString() const;

    void handleNewConnectionCallback();
    void handleMessageCallback();
    void handleCloseCallback();
private:
    InetAddress getLocalAddress();
    InetAddress getPeerAddress();

private:
    Socket _sock;
    SocketIO _sockIO;
    InetAddress _localAddr;
    InetAddress _peerAddr;
    EventLoop * _loop;

    TcpConnecitonCallback _onConnection;
    TcpConnecitonCallback _onMessage;
    TcpConnecitonCallback _onClose;
};


}//end of namespace wd


#endif

