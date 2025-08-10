#include "mysql_cilent_teacher.hpp"

using std::string;

int main(int argc,char** argv){

    const string& host = "localhost";
    const string& user = "root";
    const string& passwd = "1234";
    const string& db = "homework";
    unsigned short port = 0; 
    //默认端口为0，表示使用默认端。并且为了少使用魔法数字，所以这里直接写为了0

    MYSQL conn;

    wd::MySQLClient mysqlClient;
    mysqlClient.connect(host, user, passwd, db,port);

    mysqlClient.readOperationQuery("SELECT * FROM student");

    return 0;
}