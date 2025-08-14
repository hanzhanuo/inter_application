#ifndef __LOGINSERVICE_HPP__
#define __LOGINSERVICE_HPP__  


/*
思路分析：
packet是用来存储对TLV进行解析算法之后，所存下的包的
那在参数列表中传入packet就有原因了

那么一定要传入具体是哪个TCP请求的原因是什么？
对于整个网络连接的逻辑梳理————————这个直接决定了TCP连接为什么是被大家共享的

之前说的一对多不需要TCPconnection的主要面向的问题是——————一个ecentloop中存储着多个tcp链接
所以现在是不是也可以通过map的方式进行存储mysql连接呢？  
即我把mysql连接和tcp链接进行类比，然后把连接池和eventloop进行类比

(下面这种说法大错特错!!!)这样感觉甚至都不需要queue了，直接使用map的下标进行比较就可以
必须要用queue进行存储，否则就是没有实际存储也没有实际取出


一定要能找到为什么被大家共享，才能想明白mysql的传入该怎么写
*/

#include "Message.hpp"
#include "TcpConnection.hpp"

namespace wd
{


class User_Login_Task{

    /*
    这里我的思路是实现在某中类型的任务中，有哪些具体细分的任务，写为不同的process存到同一种类型中即可
    这样写好处是能有效避免申请类似的类的代码量
    然后在查找的时候也是类似于二次查找，所以查找效率会更高一些
    */



};

class UserLoginSection1
{
public:
    UserLoginSection1(const TcpConnectionPtr &conn, const Packet &packet)
        : _conn(conn), _packet(packet) {}
    //处理登录请求
    void process();  

private:
    void getSetting(string &setting, const char *passwd);

private:
    TcpConnectionPtr _conn;
    Packet _packet;
};

class UserLoginSection2
{
public:
    UserLoginSection2(const TcpConnectionPtr &conn, const Packet &packet)
        : _conn(conn), _packet(packet) {}
    //处理登录请求
    void process();  

private:
    TcpConnectionPtr _conn;
    Packet _packet;
};

class UserRegisterSection1
{
public:
    UserRegisterSection1(const TcpConnectionPtr &conn, const Packet &packet)
        : _conn(conn), _packet(packet) {}
    //处理登录请求
    void process();  

private:
    TcpConnectionPtr _conn;
    Packet _packet;
};

class UserRegisterSection2
{
public:
    UserRegisterSection2(const TcpConnectionPtr &conn, const Packet &packet)
        : _conn(conn), _packet(packet) {}
    //处理登录请求
    void process();  

private:
    TcpConnectionPtr _conn;
    Packet _packet;
};

class CommonMessageTask
{
public:
    CommonMessageTask(const TcpConnectionPtr &conn, const Packet &packet)
        : _conn(conn), _packet(packet) {}
    //处理登录请求
    void process();  

private:
    TcpConnectionPtr _conn;
    Packet _packet;
};



}//end of namespace wd

#endif