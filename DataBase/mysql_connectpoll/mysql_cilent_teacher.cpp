#include "mysql_cilent_teacher.hpp"



using std::cout;
using std::endl;

namespace ghz
{

MyLogger& logger = MyLogger::getInstance();

MySQLClient::MySQLClient()
{

    /*
    以后写代码都
    */
    
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
             const string & user,
             const string & passwd,
             const string & db,
            unsigned short port)
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
    logger.info("开始执行readOperationQuery");
    DEBUG_LOG("开始执行readOperationQuery");
    int ret = mysql_real_query(&_conn, sql.c_str(), sql.size());
    
    //如果是错误的情况，就直接返回一个临时对象,这个做法我也是完全没想到，以后这种必须要return内容的，就直接使用这个临时对象进行return
    if(ret != 0) {
        printf("(%d, %s)\n", mysql_errno(&_conn), mysql_error(&_conn));
        return vector<vector<string>>();
        DEBUG_LOG("readOperationQuery 出现错误");
    }
   


    //看看这里的连续两个错误判断对应着我的代码中的哪里
    vector<vector<string>> queryRes;;
    MYSQL_RES * res = mysql_store_result(&_conn);
    if(!res) {
        printf("(%d, %s)\n", mysql_errno(&_conn), mysql_error(&_conn));
        return vector<vector<string>>();
        DEBUG_LOG("readOperationQuery 出现错误");
    }
    //上面是没有结果集的错误判断


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
    DEBUG_LOG("readOperationQuery 获取字段完成");

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
    DEBUG_LOG("readOperationQuery 获取结果集数据完成");

    mysql_free_result(res);
    return queryRes;
}

/*
对结果集进行打印操作(这种得到vector的内容和打印vector的内容分离的操作，一定一定要会)
我就是不够解耦把获取内容和打印内容放到一起了
*/
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

}//end of namespace ghz