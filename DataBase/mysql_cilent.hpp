#ifndef __MysqlCilent_H__
#define __MysqlCilent_H__

#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>  //mysql的客户端头文件
#include <string>
#include <string.h>
#include <vector>

namespace apion{

using std::string;
using std::vector;
    
class MySQLClient{
    private:
    //什么时候才需要数据成员：只有在进行跨函数操作的时候才需要数据成员
    //所以是先设计行为，然后根据行为知道了什么行为封装到什么函数中，然后最后进行跨函数操作的时候，才是需要设计为数据成员的
    //然后再由数据成员反向思考构造函数需要如何初始化

   MYSQL m_conn;
   MYSQL * mp_conn;
   MYSQL_RES * m_res;

    public:

    MySQLClient(MYSQL conn);
    ~MySQLClient();   
    //我明白为什么不需要断开链接的成员函数了——————因为一定要有这种RAII的思想
    //看到这种善后操作，就一定要立刻想到析构函数。我之前对于善后和析构函数的关系还是思考的太浅了

    void Init();  //为什么初始化操作不能放在构造函数中实现？
    //但是init和connect又都需要conn，所以我的设计方式还不完善

    //也不需要错误判断，所以我认为这个函数是没有意义的封装



    void connect(const string & host,const string & user,
                 const string & passwd,const string & db,
                 unsigned short port);

    
    //void query(const string & sql_command);
    /*
    这个函数初衷还可以，但是用处不大，只是用来封装几步操作的，并且还需要设计socket指针为输入输出函数。所以设计它为行为的意义不大
    即在成员函数中，尽可能让所有的行为都是有完整作用的。这里就不是一个完整作用，而是仅仅对某个行为的一部分步骤进行了封装
    所以以后在决定设计行为的时候，最优设计一定是：对于一个完整的步骤进行封装，才是一个好的成员函数。
    或者对于一个进行一半的行为操作，如果想要封装，一定要封装在private的内部，不要暴露在public中
    */

   //写操作封装(INSERT/UPDATE/DELETE)
   bool writeOperationQuery(const string & sql_command);

   //读操作封装(SELECT)
   vector<vector<string>> readOperationQuery(const string &sql_command);
   //这种把展示结果作为容器展示出来的思想我真是一开始没想到,所以我的容器的思维局限性还是太强了
   //尝试使用map来实现二维数组？这个方法似乎不现实，所以map只适合用来处理树的二维性，只要是二维数组，就还是需要使用vector来实现，没有任何可替代性

   };
}


#endif //__MysqlCilent_H__





