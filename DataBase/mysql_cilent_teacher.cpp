#include "mysql_cilent.hpp"
#include <iostream>
#include <mysql/mysql.h>


using std::cout;
using std::endl;

namespace apion
{

MySQLClient::MySQLClient()
{
    MYSQL * pconn = mysql_init(&_conn);
    if(!pconn) {
        cout << "mysqlclient init error" << endl;
    }
}

MySQLClient::~MySQLClient()
{
    mysql_close(&_conn);
}


bool MySQLClient::connect(const string & host,
             unsigned short port,
             const string & user,
             const string & passwd,
             const string & db)
{
    MYSQL * pconn = mysql_real_connect(&_conn,
                                       host.c_str(),
                                       user.c_str(),
                                       passwd.c_str(),
                                       db.c_str(),
                                       port,
                                       nullptr,
                                       0);
    if(!pconn) {
        printf("(%d, %s)\n", mysql_errno(&_conn), mysql_error(&_conn));
        return false;
    } else 
        return true;
}

//执行INSERT/UPDATE/DELTE语句
bool MySQLClient::writeOperationQuery(const string & sql)
{
    int ret = mysql_real_query(&_conn, sql.c_str(), sql.size());
    if(ret != 0) {
        printf("(%d, %s)\n", mysql_errno(&_conn), mysql_error(&_conn));
        return false;
    } else {
        printf("Query OK, %ld row(s) affected.\n", mysql_affected_rows(&_conn));
        return true;
    }
}

//执行select语句的查询
vector<vector<string>> MySQLClient::readOperationQuery(const string & sql)
{
    cout << "1111" << endl;
    int ret = mysql_real_query(&_conn, sql.c_str(), sql.size());
    if(ret != 0) {
        printf("(%d, %s)\n", mysql_errno(&_conn), mysql_error(&_conn));
        return vector<vector<string>>();
    }
    cout << "2222" << endl;

    vector<vector<string>> queryRes;;
    MYSQL_RES * res = mysql_store_result(&_conn);
    if(!res) {
        printf("(%d, %s)\n", mysql_errno(&_conn), mysql_error(&_conn));
        return vector<vector<string>>();
    }
    //有结果集的情况
    int rows = mysql_num_rows(res);
    if(rows == 0) {
        return vector<vector<string>>();
    }
    queryRes.reserve(rows + 1);

    int cols = mysql_num_fields(res);
    printf("rows: %d, cols: %d\n", rows, cols);

    MYSQL_FIELD * filed = nullptr;
    vector<string> fileds;
    while((filed = mysql_fetch_field(res)) != nullptr) {
        fileds.push_back(filed->name);
    }
    queryRes.push_back(std::move(fileds));//转移到容器中
    cout << "333" << endl;
                                          
    MYSQL_ROW row;
    while((row = mysql_fetch_row(res)) != nullptr) {
        vector<string> tmp;
        tmp.reserve(cols);
        for(int i = 0; i < cols; ++i) {
            if(row[i] != NULL) {
                tmp.push_back(string(row[i]));
            } else  {
                tmp.push_back(string());
            }
            //cout << "i:" << i << endl;
        }
        //获取一行数据，加入到结果集中
        queryRes.push_back(tmp);
    }
    cout << "444" << endl;

    mysql_free_result(res);
    return queryRes;
}

//对结果集进行打印操作
void MySQLClient::dump(const vector<vector<string>> & res)
{
    cout << "result set has " << res.size() << " row(s)" << endl;
    for(auto & vec : res) {
        for(auto & filed : vec) {
            cout << filed << "\t";
        }
        cout << endl;
    }
    cout << endl;
}

}//end of namespace wd