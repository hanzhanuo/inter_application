#include "TcpConnection.hpp"
#include "TcpServer.hpp"
#include "Threadpool.hpp"
#include <iostream>

using std::cout;
using std::endl;
using group_6::Mytask;

//创建了线程池对象
group_6::Threadpool threadpool(4, 10);



//把组合设计模式用到了极致
class EchoServer
{
public:
    EchoServer(unsigned short port, const string ip,
               int threadNum, int queSize)
    : _threadpool(threadNum, queSize)
    , _server(port, ip)
    {
        printf("EchoServer()\n");
        using namespace std::placeholders;
        _server.setAllCallbacks(
                std::bind(&EchoServer::onConnection, this, _1),
                std::bind(&EchoServer::onMessage, this, _1),
                std::bind(&EchoServer::onClose, this, _1));
    }

    void start()
    {
        _threadpool.start();
        _server.start();
    }

private:
    void onConnection(group_6::TcpConnectionPtr conn)
    {
        cout << conn->toString() << " has connected successfully.\n";
    } 

    void onMessage(group_6::TcpConnectionPtr conn)
    {
        //onMessage函数是执行在IO线程中的
        //在执行该函数对象的过程中，时间都不宜过长，
        //否则会影响并发的执行
        //read
        string msg = conn->receive();//不能阻塞
        cout << "recv: " << msg << endl;

        //decode
        //compute   //业务逻辑的处理时间不宜过长
        //encode    //否则都会造成并发处理不能保证实时性
        //由于业务逻辑的处理过程是很复杂的，那么就可以将其交给
        //计算线程来执行具体的处理流程
        Mytask task(msg, conn);
        _threadpool.addTask(std::bind(&Mytask::process, task));
        //当计算线程处理完毕之后，再交给IO线程进行发送
        
        //假设执行的回显服务
        //string response = msg;
        //send
        //conn->send(response);//时间不宜过长
    }

    void onClose(group_6::TcpConnectionPtr conn)
    {
        cout << conn->toString() << " has closed." << endl;
    }
private:
    //线程池和Reactor（TcpServer）可以直接作为组件使用
    group_6::Threadpool _threadpool;
    group_6::TcpServer  _server;

};

int main()
{
    EchoServer server(8000, "0.0.0.0", 4, 10);
    server.start();

    return 0;
}

