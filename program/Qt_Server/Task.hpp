#ifndef __Task_H__
#define __Task_H__

#include <functional>

using std::endl;
using std::cout;

namespace group_6
{

using Task=std::function<void()>;



class Mytask
{
public:
    Mytask(const string & msg, group_6::TcpConnectionPtr conn)
    : _msg(msg)
    , _conn(conn)
    {}

    //process成员函数的执行是在线程池中的某一工作线程完成的
    void process()
    {
        cout << "Mytask::process is running" << endl;
        //decode
        //compute
        //encode
        string response = _msg;
        //send, 发送消息的操作一定要给予一个Tcp连接来完成
        //但该操作不能直接在计算线程中完成，必须要通知
        //IO线程，由IO线程最终完成数据的发送
        //_conn->send(response);
        _conn->sendInLoop(response);
    }

private:
    string _msg;
    group_6::TcpConnectionPtr _conn;
};

}//end of namespace group_6



#endif

