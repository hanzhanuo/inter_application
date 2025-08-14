#ifndef __EventLoop_H__
#define __EventLoop_H__

#include "TcpConnection.hpp"
#include "MutexLock.hpp"
#include <sys/epoll.h>

#include <vector>
#include <map>
#include <memory>
using std::vector;
using std::map;
using std::shared_ptr;


namespace group_6
{

class TcpConnection;
using TcpConnectionPtr=shared_ptr<TcpConnection>;
using TcpConnecitonCallback=function<void(TcpConnectionPtr)>;
using Functor=function<void()>;

class Acceptor;//类的前向声明，不需要头文件

class EventLoop
{
public:
    EventLoop(Acceptor&);
    ~EventLoop();
    void loop();
    void unloop();//退出循环, 要与loop函数运行在不同的线程
    void runInLoop(Functor && cb);
    void setAllCallbacks(TcpConnecitonCallback && cb1, 
                         TcpConnecitonCallback && cb2,
                         TcpConnecitonCallback && cb3)
    {
        //要表达完整的移动语义，就需要使用std::move
        //将左值(cb1,cb2,cb3)转换成右值
        //std::move的作用是显式将一个左值转换成右值
        _onConnection = std::move(cb1);
        _onMessage = std::move(cb2);
        _onClose = std::move(cb3);
    }
private:
    void waitEpollFd();
    void handleNewConnection();
    void handleMessage(int);

    int createEpollFd();
    void addEpollReadEvent(int);
    void delEpollReadEvent(int);

    int createEventfd();
    void handleReadEvent();
    void wakeup();
    void doPendingFunctors();
private:
    int                         _epfd;
    int                         _eventfd;
    bool                        _isLooping;
    Acceptor &                  _acceptor;
    vector<struct epoll_event>  _evtArr;
    map<int, TcpConnectionPtr>  _conns;
    TcpConnecitonCallback       _onConnection;
    TcpConnecitonCallback       _onMessage;
    TcpConnecitonCallback       _onClose;

    vector<Functor>             _pendingFunctors;
    //所以这个_pendingFunctors是用来存储现在都有什么任务需要处理的
    //然后在doPendingFunctors中会依次执行这些任务

    MutexLock                   _mutex;
};


}//end of namespace group_6


#endif

