#ifndef __MySQLClient_H__
#define __MySQLClient_H__

#include <mysql/mysql.h>

#include <string>
#include <vector>
using std::string;
using std::vector;

namespace wd
{
class MySQLClient
{
public:
    MySQLClient();
    ~MySQLClient();
    
    bool connect(const string & host,                 
                 const string & user,
                 const string & passwd,
                 const string & db,
                unsigned short port);
    bool writeOperationQuery(const string & sql);
    vector<vector<string>> readOperationQuery(const string & sql);

    static void dump(const vector<vector<string>> & res);

private:
    void init();

private:
    MYSQL _conn;
};

}//end of namesapce wd
#endif