#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>  //mysql的客户端头文件


int main(int argc,char** argv){

    MYSQL conn;
    MYSQL* p_conn=mysql_init(&conn);
    //必须要进行这冗余的两步，否则就会导致指针没有指向的悬空指针段错误问题


    const char* host="localhost";
    const char* user="root";
    const char* passwd="1234";
    const char* db="homework";



    /*
    助记为先写这句链接，然后才是写上面的这些字段,
    并且注意在最开始创建connect的时候，参数为conn取地址，然后返回值才为p_conn，这就是两者之间需要格外注意的关系
    所以在助记为在第一步建立链接的时候必须使用原始对象，不能使用指针
    所以mysql编程其实就是把像TCP链接一样的核心步骤的函数记住，然后再传入原本命令行参数所需要的内容即可
    所以这个其实本质上就像在写配置文件一样。
    */
   /*
   关于配置文件：配置文件只负责存信息，不负责进行任何操作
   操作都是程序读配置信息然后再对读取的内容进行操作的
   所以上面的这一片char* 可以写在配置文件中，这就是最终版本的mysqlC语言编程
   */
    p_conn=mysql_real_connect(&conn,host,user,passwd,db,0,nullptr,0);
    /*
    建立连接的函数只有real这一种，进行查询的时候才是有query和real两种
    但是query也是一般来说使用real版本的
    */

    if(!p_conn){   //此时表示为nullptr，所以此时是错误
                   //(基本上if都是用来写错误判断的，记住这条原则就会写if的条件了)
       fprintf(stderr,"%s\n",mysql_error(&conn));
       //这里的报错还是conn&的报错，后面connect连接成功之后，报错就都用p_conn来承接了(一定要记住这个区别)

       return EXIT_FAILURE;
       
    }

    //上面是所有操作都要进行的无脑链接操作，然后最后面的断开连接与之呼应，也是需要无脑先写出来，并且写的位置也是固定在return的前面一条语句的

#if 1    //这是进行删除语句的使用(即所有的写操作都是不需要进行获取查询结果的操作的)

    const char* sql_1="DELETE FROM student WHERE s_id=9";
    /*
    query是对于SQL命令行的查询，所以所有的命令行执行都是使用的这个query语句来实现的
    然后一定要使用这种real版本，因为这个版本可以避免命令行中有/0导致的半包读入的问题
    */
    int ret=mysql_real_query(p_conn,sql_1,strlen(sql_1));
    
    if(ret!=0){   //if一定是错误判断，所以反推这里的错误一定是只要不为0就是出错了
        fprintf(stderr,"ERROR %d:%s\n",
                mysql_errno(p_conn),
                mysql_error(p_conn));
        /*
        一定要学会这种错误类型和错误内容一起打印的操作
        并且还一定要学会这种长语句换行写，这样一定一定是对自己阅读代码有利的写法
        */

        return EXIT_FAILURE;
    }

    //过了if判断，就是表明操作成功了。此时就返回操作成功的语句(还是模仿的sql的语句写的)
    printf("QUERY OK,%ld row affected.\n",mysql_affected_rows(p_conn));

#endif

    const char* sql_2="INSERT INTO student(s_id,s_sex) VALUE(4,'男')";
    
    int ret2=mysql_real_query(p_conn,sql_2,strlen(sql_2));  
    /*
    上下这两条语句还不熟练:这两条作为无论是读操作还是写操作都需要使用的语句是需要极其极其熟练的
    因为这两条语句没有任何的变化，纯纯是一种僵化的操作
    */
    
    if(ret2!=0){
        fprintf(stderr,"ERROR %d:%s\n",
                mysql_errno(p_conn),
                mysql_error(p_conn));
        return EXIT_FAILURE;
    }

    //这个操作成功的紧接着写的话也是需要直接记住的，完全没有任何区别的操作
    printf("QUERY OK,%ld row affected.\n",mysql_affected_rows(p_conn));




    //紧邻着return的关闭链接操作
    mysql_close(p_conn);
    return 0;
}