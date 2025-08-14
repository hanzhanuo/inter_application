#include "mysql_cilent_teacher.hpp"
#include  "mysql_connpoll.hpp"

using std::string;



int main(int argc,char** argv){

    const string& host = "localhost";
    const string& user = "root";
    const string& passwd = "1234";
    const string& db = "homework";
    unsigned short port = 0; 
    //默认端口为0，表示使用默认端。并且为了少使用魔法数字，所以这里直接写为了0

    MYSQL conn;

    // ghz::MySQLClient mysqlClient;
    // mysqlClient.connect(host, user, passwd, db,port);
    


    string sql_command = "SELECT * FROM student";
    mysqlClient.readOperationQuery(sql_command);


    vector<vector<string>> res = mysqlClient.readOperationQuery(sql_command);
    ghz::MySQLClient::dump(res); 

    return 0;
}