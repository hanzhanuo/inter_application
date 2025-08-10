#include "mysql_cilent_teacher.hpp"
#include <mysql/mysql.h>

using std::string;

int main(int argc,char** argv){

    const string& host = "localhost";
    const string& user = "root";
    const string& passwd = "1234";
    const string& db = "homework";
    unsigned short port = 0; //默认端口为0，表示使用默认端

    MYSQL conn;

    wd::MySQLClient mysqlClient{};  
    /*
    在写创建对象的时候，如果构造函数是无参的，那么就不能使用小括号定义对象。
    否则就会导致识别为声明一个成员函数

    但是如果一定想要写，可以通通改为写为花括号，这样花括号中既可以不传参数，又可以在调用有参构造函数的时候传对应的参数

    */
    mysqlClient.connect(host, user, passwd, db,port);

    mysqlClient.readOperationQuery("SELECT * FROM student");

    return 0;
}