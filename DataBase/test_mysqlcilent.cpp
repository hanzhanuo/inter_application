#include "mysql_cilent.hpp"

using std::string;

int main(int argc,char** argv){

    const string& host = "localhost";
    const string& user = "root";
    const string& passwd = "1234";
    const string& db = "homework";
    unsigned short port = 0; //默认端口为0，表示使用默认端

    MYSQL conn;

    apion::MySQLClient mysqlClient(conn);
    mysqlClient.connect(host, user, passwd, db, port);

    mysqlClient.readOperationQuery("SELECT * FROM student");

    return 0;
}