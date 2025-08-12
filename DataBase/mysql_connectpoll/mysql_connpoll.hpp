#ifndef __Connectpool_H__
#define __Connectpool_H__


/*
思路：完全模仿线程池，首先要有一个命令行队列然后又要有一个连接池
然后还要实现加锁的操作，然后还要对于加锁操作实现读操作加锁和写操作加锁的统一
*/


#include "mysql_cilen_teacher.hpp"
#include "CommandQueue.hpp"
#include <vector>
#include <memory>

namespace ghz{

using Connect=ghz::MySQLClient;

using std::vector;
using std::unique_ptr;

class ConnPoll{
    private:
        vector<unique_ptr<Connect>>  _connects;
        int                         _connectNum;
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

        
};

}

#endif //__Connectpool_H__