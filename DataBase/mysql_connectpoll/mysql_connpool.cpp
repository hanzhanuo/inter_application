#include "mysql_connpoll.hpp"

namespace ghz{

    ConnPoll::ConnPoll(int connectNum,int queSize)
    : _connectNum(connectNum)
    , _connects()
    , _queSize(queSize)
    ,_commandque(_queSize)
    , _isExit(false)
    {
        _connects.reserve(_connectNum);

        //将现有的所有链接插入到连接池中(即最开始初始化的连接数量)
        for(int i=0;i<_connectNum;++i)
        {
            
            _connects.emplace_back(std::make_unique<Connect>());
        }
    }

    //让连接池中所有的链接开始工作
    //向链接容器中存放链接，并且启动链接
    //使用的方法是mysql类型的智能指针，调用的是链接函数
    
    void ConnPoll::start()
    {
        for(auto & conn : _connects)
        {
            conn->start();
        }
    }

    //让连接池中所有的链接停止工作
    void ConnPoll::stop()
    {
        _isExit = true; //设置退出标志位
        for(auto & conn : _connects)
        {
            conn->stop();
        }
    }

    void ConnPoll::addCommand(Command && cb)
    {
        _commandque.push(std::move(cb));
    }

    void ConnPoll::doCommand(){
        while(!_isExit) {
        //每一个子线程都要从任务队列中获取任务，并执行
        Command command = _commandque.pop();
        if(command) {
            //执行任务
            command();

            
            
          }
       }
    }

    
}