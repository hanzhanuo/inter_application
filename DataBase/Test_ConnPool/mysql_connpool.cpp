#include "mysql_connpoll.hpp"

#include <iostream>


namespace ghz{

    using group_6::Condition;
    using group_6::MutexLockGuard;


    ConnPool::ConnPool(const string &host,
                unsigned short port,
                const string &user,
                const string &password,
                const string &db,
                int connectNum)
    : _connectNum(connectNum)
    ,_connects()
    ,_connMutex()
    ,_notEmpty(_connMutex)   //这两者的传参具体的锁我总是忘
    ,_notFull(_connMutex)
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

    ConnPool::~ConnPool()
    {
        //析构函数用于关闭所有链接。首先进行上锁操作，否则可能导致正在执行任务就被关闭了
        MutexLockGuard autolock(_connMutex);
        
        while(!_connects.empty()) {
            auto conn = _connects.front();
            _connects.pop();
            conn->~MySQLClient();  
            //因为这个析构函数是手动进行close客户端程序的mysql连接的操作
            //所以这里手动调用,就是在实现关闭所有链接的操作了
            
        }
    }


    bool ConnPool::empty() const{

        //MutexLockGuard autolock(_connMutex);
        //const函数内部没法对任何内容进行改变，所以没法加锁？
        //这两个因果关系是什么原理？

        return _connects.empty();
    }
    bool ConnPool::full() const{

        
        return _connects.size() == _connectNum;
    }

    size_t ConnPool::avaiable() const{

        size_t cnt=_connectNum-_connects.size();

        std::cout<<"可用链接数为"<<cnt<<std::endl;

        return cnt;
    }

    

    
     Conn_Elem ConnPool::get_connect()
    {

        Conn_Elem tmp;   //用于承接返回值的临时元素变量

        //对连接池上锁
        MutexLockGuard autolock(_connMutex);

        //进行que的判断
        while(empty()) {
            _notEmpty.wait();
        }

        //如果条件判断没的可拿了就进行条件变量的wait操作

        tmp=_connects.front();
        _connects.pop();  //释放应该比这个还麻烦些，应该设计到shared_ptr的归还操作，然后还有归还到queue中
        //感觉可以使用一个tmp?

        //对条件变量进行notify_one的操作中
        _notFull.notifyOne();

        std::cout<<"已获取一个连接"<<std::endl;

        return tmp;
    }

    //归还某个链接的操作
    void ConnPool::release_connect(Conn_Elem elem)
    {

        //先加锁再判断
        MutexLockGuard autolock(_connMutex);

        //进行que的判断
        while(full()) {
            _notFull.wait();
        }

        _connects.push(elem);  //释放应该比这个还麻烦些，应该设计到shared_ptr的归还操作，然后还有归还到queue中
        //感觉可以使用一个tmp?

        //对条件变量进行notify_one的操作中
        _notEmpty.notifyOne();

        std::cout<<"已归还一个连接"<<std::endl;
    }

   

    
}