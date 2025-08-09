#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>  //mysql的客户端头文件


int main(int argc,char** argv){

    MYSQL conn;
    MYSQL* p_conn=mysql_init(&conn);  
    /*
    一定要百分百记住了必须要这样写，写成别的样子就会导致段错误
    一定一定要写成这种脱裤子放屁的写法
    */

    const char* host="localhost";
    const char* user="root";
    const char* passwd="1234";
    const char* db="homework";
    p_conn=mysql_real_connect(&conn,host,user,passwd,db,0,nullptr,0);
    /*
    注意：这里connect成功之后，获取的这个内容是一个socket
    ——————因此一定要记住sql的写法也是一种套接字通信，所以要把它和TCP之类的放在一块去思考
    那不就是mysql连接池在做的事吗... 所以我认为连接池就是创建许多链接，这些连接返回的socket再分别进行读写操作
    所以这样看就能理解它和线程池之间的相似之处了
    */

    if(!p_conn){
        fprintf(stderr,"%s\n",mysql_error(p_conn));
        //对于printf来说，一定是上来就写为双引号，fprintf最上来要写为是输出到三个标准缓冲区中的哪个
        //然后一般使用fprintf了，就都是输出到标准错误中的。基本上可以记住这个规律了

        //这里不写errno，应该是因为对于connect的时候没有错误编号吧，其余的标准错误应该都是要写错误编号的

        return EXIT_FAILURE;
    }

    //进行select操作就是在进行读操作，以后记住了这种说法就可以了
    const char* sql="SELECT* FROM student";
    printf("exec 1st round read query\n");
    /*
    只有在进行读操作的时候，才需要先获取结果集之后，再进行查询
    结果集就是获取了MYSQL_ROW之后，所以在这条语句之后，才能进行进一轮的select操作
    但是进行修改的写操作是可以连续进行的(仅限在同一个服务器内可以连续进行，如果是多个服务器就要考虑写冲突的问题了)
    */

    int ret=mysql_real_query(p_conn,sql,strlen(sql));
    //无论是读写的函数，都是使用的这个函数，所以这个函数更是要记的极其牢固了
    if(ret!=0){
        fprintf(stderr,"ERROR %d:%s\n",
                mysql_errno(p_conn),
                mysql_error(p_conn));
        return EXIT_FAILURE;
    }

    //这里才是读操作的重头戏
    //因为写操作没有什么要展示的，所以没什么可写的，而读操作是必须要隆重展示的，所以可写的内容非常多

    MYSQL_RES* res=mysql_use_result(p_conn);
    /*
    这个获取查询结果有use版本和store版本。
    store版本是全部读入客户端中，
    use版本是先不读进来，然后在mysql_fetch_row的时候再进行
    mysql_fetch_row就是在获取结果集。所以use和不能重复查询的需要注意的位置是同一个位置
    */

    if(res){   //这里比较特殊：这里的if是获取成功的情况。这个点要特殊的注意

        //获取行数和列数，为后面的遍历结果做准备
        //这里最好的写法是加打印，来确定自己最终打印结果的行数列数是否正确

        int cols=mysql_num_fields(res);   //注意没有columns函数，只有获取字段数量的函数
        int rows=mysql_num_rows(res);

        printf("rows:%d,cols:%d",rows,cols);

        /*
        重头戏：获取结果集。只有在获取完结果集之后，才能进一步进行第二轮的读(select)操作
        下面这两部分代码都是在获取结果集——————所以获取结果集并不是一个固定的操作，而是行和列两部分获取的操作
        即所有的fecth操作都是在获取结果集的操作
        */

        MYSQL_FIELD* parr=mysql_fetch_fields(res);
        for(int i=0;i<cols;i++){
            printf("%s\t",parr[i].name);
            /*
            \t是空格，name是在获取字段名
            */
        }
        printf("\n");

        //其实获取字段名严格来说也不算获取数据，只有这里进行每行的获取的时候，才是真正的获取数据了
        //并且一定要注意这个易错点：对于row的返回值不是指针类型，对于field的返回值是指针类型。这个不要写错了
        MYSQL_ROW row;  
        //这里之所以不像上面的获取字段一样一步到位，是因为这里要对这个变量进行while循环处理
        //所以while循环总不能每次都进行一次这个语句的定义吧

        while((row=mysql_fetch_row(res))!=nullptr){
            /*
            所以这种双等号的写法在while中也是被允许的，所以以后要学会使用这种双等号
            并且这里的双等号是不得不使用的，原因是此时while循环外面没办法给row赋值，所以只能通过这种双等号的方式实现row的赋值操作
           

            并且注意：其实这里的双等号也并不是双等号，而是在进行小括号包裹起来的双等号。
            所以我自认为的直接双等号的写法是错的
            */

            for(int i=0;i<rows;i++){
                printf("%s\t",row[i]);   
                //注意这里是row的下标，而不是rows的下标，rows只是个数字而已。这个问题非常非常容易错，一定要小心
            }
            printf("\n");
        }
        

    }else{    //总算知道为什么这么写好了：这样写能快速找到和其直接对应的else。所以在大型代码中，这样写会让代码更有可读性
        fprintf(stderr,"ERROR %d:%s\n",
                mysql_errno(p_conn),
                mysql_error(p_conn));
    }

    mysql_free_result(res);
    //所以第二次读取最好最好是在释放了结果集之后再进行读取吧，感觉这是最好的二次读取的位置了

    mysql_close(p_conn);
    return 0;
}