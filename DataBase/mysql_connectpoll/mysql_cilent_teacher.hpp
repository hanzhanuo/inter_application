#ifndef __MySQLClient_H__
#define __MySQLClient_H__

#include "func.h"


using std::string;
using std::vector;

namespace ghz
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

}//end of namespace ghz
#endif