#include "mysql_cilent.hpp"

namespace apion{

    #if 0
    //这个构造函数的写法是错的：因为在构造函数中不能直接调用库函数，原因也不是临时变量的原因，而是就不能调用
    MySQLClient::MySQLClient(MYSQL conn)
    :m_conn(conn)
    ,mp_conn(mysql_init(&m_conn))
    //,m_res(mysql_use_result(mp_conn))
    //这个不应该在构造函数进行初始化，原因是这个函数是需要进行错误判断的，而上面的init函数是不需要错误判断的
    //所以只有不需要错误判断的函数才可以在初始化列表中被调用，否则就只能一开始不初始化，在后续操作的时候再进行调用并初始化
    {

    }
    #endif

    MySQLClient::~MySQLClient(){
        mysql_close(mp_conn);

        //需不需要对于m_conn进行销毁？
    }

    void MySQLClient::connect(const string & host,const string & user,
                 const string & passwd,const string & db,
                 unsigned short port)
                 {

                    /*
                    我认为需要把conn设置为全局的数据成员，也是因为这个函数需要
                    ——————即当在封装后，在封装函数中仍然有多个函数需要，那就需要设置为数据成员
                    所以设计不同行为(成员函数)的封装是我设计数据成员的基础，设计数据成员又是我设计构造和析构的基础
                    */
                    mp_conn=mysql_real_connect(&m_conn,host.c_str(),user.c_str(),passwd.c_str(),db.c_str(),port,nullptr,0);

                    /*
                    封装这个类时候的需要注意的点：
                    1.这个函数中间有个real，我总是会把这个给漏掉
                    2.这个传入的是string是对的——————因为这个sql本身是个C语言中的库，这里把它封装为cpp中的类，就必须要进行入乡随俗的强转
                    并且时刻记住string类型的强转的助记：string是char*的高版本，谁是高版本谁就可以兼容，低版本就没法兼容,必须要进行强转操作(除了这个函数，还可以使用static_cast进行强转)
                    3.这个函数需要用socket指针来承接，并且需要对这个指针进行错误判断。我总是会忘了把错误判断封装进来
                    */

                    if(!mp_conn){
                        fprintf(stderr,"%s\n",mysql_error(mp_conn));

                        exit(1);    //这里没法return，所以我只能修改为使用错误退出的函数了
                    }
                    
                 }

    bool MySQLClient::writeOperationQuery(const string & sql_command){
        int ret=mysql_real_query(mp_conn,sql_command.c_str(),strlen(sql_command.c_str()));
        //原来strlen也是需要转为char* 的啊，我之前一直不知道。以后一定要多练多熟练

        if(ret!=0){  //非0表示错误情况
            fprintf(stderr,"ERROR%d:%s\n"
                    ,mysql_errno(mp_conn)
                    ,mysql_error(mp_conn));

            return ret;
            /*
            其实这个返回值非零表示错误，就立刻想到设计为bool类型，这个我也是完全没想到
            所以以后我一定要能立刻想到有错误处理的就直接和返回值为bool联想起来
            这种思维定式一定要熟练
            */
        }

        //在正确情况下不要忘了进行return，这个问题老师说过无数遍了，一定要小心这种if导致的return陷阱
        return 0;
    }



    /*
    这个函数还忘记加锁进行操作了，一定一定要记得对读操作进行加锁
    读操作加锁是解决单一客户端同时读的操作的，写操作加锁是解决多个客户端同时写的操作的
    */

    vector<vector<string>> MySQLClient::readOperationQuery(const string &sql_command){

        vector<vector<string>> tmp;   
        /*
        由于不是作为传入传出参数，所以这个数组直接作为局部变量这样放好像也可以
        这就又涉及到一个问题了：对于传入传出参数，还有全局变量和局部变量的设计
        对于面向对象设计来说，一定是只有需要在全局使用的容器才设置为数据成员
        并且一旦一个容器or一个变量设计为全局使用了，那就肯定避免不了在函数中必须要作为传入传出参数使用———————即必须设计为引用的方式进行传入函数，才可以实现函数内部实时修改

        这里因为只需要作为返回值使用，而不需要进行任何的处理
        所以这种特征就是一定要设计为局部变量——————因为在面向对象设计中，都是全局变量作为传入传出参数，局部变量作为返回值
        这样总结就能明白的多了
        */

        int ret=mysql_real_query(mp_conn,sql_command.c_str(),strlen(sql_command.c_str()));

        //这个判断是所有读写操作都要做的，但是下面的判断不是所有的读写操作都要做的
        if(ret!=0){
            fprintf(stderr,"ERROR%d:%s"
                    ,mysql_errno(mp_conn)
                    ,mysql_error(mp_conn));
        }

        m_res=mysql_use_result(mp_conn);


        if(m_res){

            int cols=mysql_num_fields(m_res);
            int rows=mysql_num_rows(m_res);
            printf("columns:%d,rows:%d",cols,rows);

            MYSQL_FIELD* parr=mysql_fetch_fields(m_res);
            for(int i=0;i<cols;i++){
                //我总是忘这个\t进行换行，这个展示方法一定要熟练，以后会有大用
                printf("%s\t",parr[i].name);
            }
            printf("\n");

            MYSQL_ROW row;
            
            int cnt=0;
            while((row=mysql_fetch_row(m_res))!=nullptr){
                cnt++;
                for(int i=0;i<rows;i++){
                    printf("%s\t",row[i]);   //因为有\t进行分隔，所以这里的操作应该是打印一整行数据出来
                    tmp[cnt][i]=row[i];   
                    /*
                    对于这个vector怎么实现赋值的操作？
                    所以这里不是赋值操作的问题，而是我没捋清楚一维和二维数组之间的关系的问题
                    这里在捋清楚关系之后，写起来果然就对了
                    */

                    /*
                    然后这里再拓展一下迭代器方法和swap方法
                    1.迭代器方法就是v2.assign(v1.begin(), v1.end())
                    ——————即通过assign这个赋值函数，实现把v1的内容全部赋值给v2

                    2.swap方法的效果是两个vector的数据完全交换，而不是一个复制另一个
                    */
                }
                printf("\n");

                //必须要在这个位置实现return的操作，但是怎么存储起来呢？  已解决✅
               
                
                

            }
            

        }else{    //这里就是获取结果集错误，进行错误判断

        }

         return tmp;    
         //这里进行return是最好的，因为这里return不管是否获取成功了，都可以进行return了
         //即所谓的进行一锤子买卖

    }
}