#include "TcpConnection.hpp"
#include "EventLoop.hpp"
#include <sstream>


namespace wd
{


TcpConnection::TcpConnection(int fd, EventLoop * loop)
: _sock(fd)
, _sockIO(fd)
, _localAddr(getLocalAddress())
, _peerAddr(getPeerAddress())
, _loop(loop) //很容易忘记
{}

TcpConnection::~TcpConnection()
{
    printf("~TcpConnection()\n");
}

string TcpConnection::receive()
{
    char buff[65536] = {0};//在栈上设置的64KB空间
    int ret = _sockIO.readline(buff, sizeof(buff));
    //设置第二个参数ret,是为了防止字符串内容中间有'\0'
    return string(buff, ret);
}

void TcpConnection::send(const string & msg)
{
    if(msg.size() > 0)
        _sockIO.sendn(msg.c_str(), msg.size());
}

void TcpConnection::sendInLoop(const string & msg)
{ 
    if(_loop) {
        _loop->runInLoop(std::bind(&TcpConnection::send, this, msg));
    }
}

//判断一个连接是否断开了
bool TcpConnection::isClosed() const
{
    char buff[20] ={0};
    return _sockIO.recvPeek(buff, sizeof(buff)) == 0;
}

void TcpConnection::shutdown()
{
    _sock.shutdownWrite();
}
    
string TcpConnection::toString() const
{
    std::ostringstream oss;
    oss << "tcp " << _localAddr.ip() << ":" << _localAddr.port()
        << " -> " << _peerAddr.ip() << ":" << _peerAddr.port(); 
    return oss.str();
}

void TcpConnection::handleNewConnectionCallback()
{
    if(_onConnection) {
        //在成员函数内部获取本对象的智能指针shared_ptr
        //shared_ptr<TcpConnection> sp(this);//error,导致智能指针的误用
        //this -> shared_ptr
        //正确方案：TcpConnection继承自辅助类std::enable_shared_from_this
        //继承之后，该类就拥有了一个成员函数shared_from_this
        _onConnection(shared_from_this());
    }
}

void TcpConnection::handleMessageCallback()
{
    if(_onMessage) {
        _onMessage(shared_from_this());
    }
}

void TcpConnection::handleCloseCallback()
{
    if(_onClose) {
        _onClose(shared_from_this());
    }
}
    
InetAddress TcpConnection::getLocalAddress()
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);
    int ret = getsockname(_sock.fd(), (struct sockaddr*)&addr, &len);
    if(ret < 0) {
        perror("getsockname");
    }
    return InetAddress(addr);
}
    
InetAddress TcpConnection::getPeerAddress()
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);
    int ret = getpeername(_sock.fd(), (struct sockaddr*)&addr, &len);
    if(ret < 0) {
        perror("getpeername");
    }
    return InetAddress(addr);
}
}//end of namespace wd
