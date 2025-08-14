

#ifndef __Connectpool_H__
#define __Connectpool_H__


/*
思路：完全模仿线程池，首先要有一个命令行队列然后又要有一个连接池
然后还要实现加锁的操作，然后还要对于加锁操作实现读操作加锁和写操作加锁的统一
*/


#include "mysql_cilent_teacher.hpp"
#include "CommandQueue.hpp"
#include "MutexLock.hpp"
#include "Condition.hpp"
#include <functional>
#include <vector>
#include <memory>
#include <queue>


namespace ghz{

using std::queue;
using std::shared_ptr;
using std::function;
using group_6::Condition;
using group_6::MutexLock;

using Connect=function<void()>;  // 连接的类型定义



class ConnPoll{
    
    public:

        ConnPoll(const string &host,
                unsigned short port,
                const string &user,
                const string &password,
                const string &db,
                int connectNum);   //这里需要修改为对统一的服务器端进行连接池，所以需要填参数为ip，端口号，相当于它传入了connect的所有参数
        //所以完全照着connect函数的参数进行构造函数的封装

        //我想的是在构造函数中调用成员函数了，然后成员函数设计为start函数



        ~ConnPoll();  //析构函数用于关闭所有链接

        // 禁止拷贝构造和赋值，防止多份连接池实例
        ConnPoll(const ConnPoll&) = delete;
        ConnPoll& operator=(const ConnPoll&) = delete;


        bool empty() const;
        bool full() const;

        void get_connect();//开启连接池的运行————————对所有的mysql进行链接操作
        void release_connect();//停止连接池的运行
        //Command是一个函数对象，用右值引用来表示
        //void addCommand(Command && cb);

    private:
        //void doCommand();//每一个子连接都要做的事儿


        /*
        行为仍然有缺陷，需要的行为：
        启动连接池
        停止连接池

        连接池还需要加锁：进行读写操作本身要加锁，然后进行
        */

        //设计数据成员：

        private:        
        int                         _connectNum;
        queue<shared_ptr<MySQLClient>>  _connects;  
        //这里存的是一个share_ptr,避免使用完了立刻就对这个musql进行释放了


        MutexLock                   _connMutex;
        Condition                   _notFull;
        Condition                   _notEmpty;
        bool                        _isExit;   //表示是否整个连接池都结束运行了
        //所以这个是在构造和析构之间来实现的

        
};

}

#endif //__Connectpool_H__