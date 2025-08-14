#include "mysql_connpoll.hpp"

namespace ghz{

    ConnPoll::ConnPoll(const string &host,
                unsigned short port,
                const string &user,
                const string &password,
                const string &db,
                int connectNum)
    : _connectNum(connectNum)
    ,_connects()
    ,_connMutex()
    , _isExit(false)   //表示此线程池是否退出了
    {

        //将现有的所有链接插入到连接池中(即最开始初始化的连接数量)
        for(int i=0;i<_connectNum;++i)
        {
            //生成一个mysql连接
            auto conn = std::make_shared<MySQLClient>();   //这里就是传进行new的时候的参数，因为不需要参数，所以就设置为空就行
            //注意这个不是临时对象，make一定是new出来真实的内容的
            conn->connect(host, port, user, password, db);
            _connects.push(conn);
        }
    }


    bool ConnPoll::empty() const{
        return _connects.empty();
    }
    bool ConnPoll::full() const{
        return _connects.size() == _connectNum;
    }


    


    //关于如何魔改start函数：启动连接池的功能应该是在构造函数中就实现了(构造函数中调用连接池中的所有链接的connect函数)
    //所以所有的关闭链接操作也应该写在析构函数中
    //然后现在需要写一个拿取链接和放回连接的函数，然后这个函数应该是涉及到连接池中还是否有连接可以拿取的加锁和条件变量
    
    
    //让连接池中所有的链接开始工作
    //向链接容器中存放链接，并且启动链接
    //使用的方法是mysql类型的智能指针，调用的是链接函数
    
    void ConnPoll::get_connect()
    {
        //对连接池上锁
        MutexLockGuard autolock(_connMutex);

        //进行que的判断
        while(empty()) {
            _notEmpty.wait();
        }

        //如果条件判断没的可拿了就进行条件变量的wait操作

        _connects.pop(conn);  //释放应该比这个还麻烦些，应该设计到shared_ptr的归还操作，然后还有归还到queue中
        //感觉可以使用一个tmp?

        //对条件变量进行notify_one的操作中
        _notFull.notifyOne();
    }

    //归还某个链接的操作
    void ConnPoll::release_connect()
    {

        //先加锁再判断
        MutexLockGuard autolock(_connMutex);

        //进行que的判断
        while(full()) {
            _notFull.wait();
        }

        _connects.push(conn);  //释放应该比这个还麻烦些，应该设计到shared_ptr的归还操作，然后还有归还到queue中
        //感觉可以使用一个tmp?

        //对条件变量进行notify_one的操作中
        _notEmpty.notifyOne();
    }

   

    
}