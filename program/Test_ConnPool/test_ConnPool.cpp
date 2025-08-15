#include "mysql_cilent_teacher.hpp"
#include  "mysql_connpoll.hpp"


#include "func.h"


using std::string;
using group_6::MutexLock;
using group_6::MutexLockGuard;

MutexLock Command_Mutex;   //切记是MutexLock，而不是MutexLockGuard


//此时连接池先进行初始化，再进行的process的addtask
//所以在这步之前必须要进行初始化，然后再进行这种类型的addtask(std::bind())
//并且通过和filestream进行对比，就能看出这个addtask可以在任何位置执行
//但是由于filestream必须要进行for循环切片，然后对切片进行添加的专门操作，然后这里connPool的就在任何位置都可以
//这就是两个类的addtask的区别  而这个区别是由每个不同的process的
//addtask可以说是对于process函数的调用，然后这里是对于process的定义。所以在调用的时候，在什么位置调用都可以，前面加类名就可以了
//所以这就是每个addtask和process的直接区别

void mysql_process(int command_type,const string& command){

    //连接池的初始化不需要进行在process中进行，然后这里只需要进行判断并取出操作即可
    ghz::Conn_Elem tmp=connPool.get_connect();

    if(!tmp){
        //获取连接失败
        return;
    }

    //根据命令类型进行不同的操作
    switch(command_type){
        case group_6::READ:  //读操作
            {
                MutexLockGuard autolock(Command_Mutex);
                tmp->readOperation(command);
            }
            break;
        case group_6::WRITE:  //写操作
            {
                MutexLockGuard autolock(Command_Mutex);
                tmp->writeOperation(command);
            }
            break;
        default:
            break;
    }

    connPool.release_connect(tmp);

}




int main(int argc,char** argv){

    

    const string& host = "localhost";
    const string& user = "root";
    const string& passwd = "1234";
    const string& db = "homework";
    unsigned short port = 0; 
    //默认端口为0，表示使用默认端。并且为了少使用魔法数字，所以这里直接写为了0

    // MYSQL conn;

    // ghz::MySQLClient mysqlClient;
    // mysqlClient.connect(host, user, passwd, db,port);

    ghz::ConnPool connPool(host, port, user, passwd, db, 10);
    


    ghz::Conn_Elem tmp=connPool.get_connect();  //获取一个线程池链接

    if(!tmp){
        //获取连接失败
        return -1;
    }

    string sql_command = "SELECT * FROM student";

    vector<vector<string>> res;
    
    {  //在语句块内部才能实现命令行执行操作
        MutexLockGuard autolock(Command_Mutex);
        res = tmp->readOperationQuery(sql_command);
    }
    
    ghz::MySQLClient::dump(res);

    connPool.release_connect(tmp);

    return 0;
}