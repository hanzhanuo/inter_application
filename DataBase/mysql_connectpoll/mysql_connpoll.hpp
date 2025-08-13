

#ifndef __Connectpool_H__
#define __Connectpool_H__


/*
思路：完全模仿线程池，首先要有一个命令行队列然后又要有一个连接池
然后还要实现加锁的操作，然后还要对于加锁操作实现读操作加锁和写操作加锁的统一
*/


#include "mysql_cilent_teacher.hpp"
#include "CommandQueue.hpp"
#include <functional>
#include <vector>
#include <memory>

namespace ghz{

using std::vector;
using std::unique_ptr;
using std::function;

using Connect=function<void()>;  // 连接的类型定义



class ConnPoll{
    private:        
        int                         _connectNum;
        vector<unique_ptr<MYSQL>>  _connects;  
        //vector中存的不是智能指针。现在的逻辑应该是让一个个链接后返回的MYSQl类型的智能指针进入这个vector中
        int                         _queSize;
        CommandQueue                _commandque;
        bool                        _isExit;   //表示是否整个连接池都结束运行了
    public:
        ConnPoll(int connectNum,int queSize);

        void start();//开启连接池的运行
        void stop();//停止连接池的运行
        //Command是一个函数对象，用右值引用来表示
        void addCommand(Command && cb);

    private:
        void doCommand();//每一个子连接都要做的事儿


        /*
        行为仍然有缺陷，需要的行为：
        启动连接池
        停止连接池
        add指令，get指令，do指令

        连接池还需要加锁
        */
        

        
};

}

#endif //__Connectpool_H__