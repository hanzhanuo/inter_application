#include "func.h"
#include <stdlib.h>
#include <asm-generic/socket.h>  //看来这个是epoll专属的头文件了，普通的server中没有使用这个头文件
#include <sys/epoll.h>


//这些进行连接的代码都是直接写在main函数中的，要进行和纯server的步骤的区分
//区分就是原本的客户端是循环进行accept，然后这里是先epoll，然后对于在epoll内部进行accept

int main(int argc,char** argv){


    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        perror("socket");
        //close(listen_fd);    这里不应该无脑关闭fd，因为此时fd还没建立成功呢
        //所以这里是唯一不需要手动关闭fd的位置
        return EXIT_FAILURE;
    }

    int on=1;
    setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    //这个语句除了第一个参数是需要根据自己来设定的，其他所有的参数都是无脑完全相同的

    //进行绑定前的结构体配置(到时候epoll也是需要在插入前进行初始化配置)
    struct sockaddr_in serveraddr;  //果然这里为了和老师的保持一致，还是使用了server来命名结构体了
    memset(&serveraddr,0,sizeof(serveraddr));
    //memset的第一个参数用于传入首地址，所以这里就使用结构体取地址实现了
    //(因为只有数组可以实现用数组名作为首地址，其他的都是必须要进行取地址操作的，没有特例)

    serveraddr.sin_family=AF_INET;
    //一定是先确定协议族(ipv4)，然后再确定ip，端口。所以这才是正确的助记顺序
    serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    serveraddr.sin_port=htons(8080);      
    
    /*
    要记住端口是数字，所以不用写字符串双引号;但是ip地址是点分十进制，所以要写双引号
    并且这里助记：无论是给人看的端口号还是给极其看的，这里都是转为s(short),只不过xtox的方向变了，仅此而已
    然后到时候ip地址的写法也变为了inet_ntoa   
    并且在进行写项目变式的时候，一定要记住这里写项目的时候写的是天明的服务器地址和端口，而不是这里这样自己填本地的写法
    */

    int ret=bind(listen_fd,(const struct sockaddr*)&serveraddr,sizeof(serveraddr));
    //这里就属bind是最麻烦的，这里的前面*是用来强转的，后面&是用来取地址的

    if(ret<0){
        perror("bind");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    ret=listen(listen_fd,20000);   //这里的第二个参数用来表示两个队列的可以存放的链接数量的总和
    if(ret<0){
        perror("listen");
        close(listen_fd);
        return EXIT_FAILURE;
    }
    //所以写了这么多遍之后，基本就可以确定所有的这种错误判断都是这样写
    //可能出现的变数就是后面如果出现了新的fd，就需要对新的fd多进行一步操作


    //然后下面的一系列操作就都是针对于epoll的操作了
    int epoll_fd=epoll_create1(0);   
    //创建函数的标志位都默认设置为0即可，这也就是为什么这个函数完全没讲细节——————因为它没有任何需要考虑的，就自动帮忙建好了
    if(epoll_fd<0){
        perror("epoll_create1");
        //注意这里的fd是关闭谁的fd:这个close的写法非常微妙：
        //由于此时还没创建出来epoll的fd，所以不能对epoll的fd进行关闭，并且又因为出错误了，所以按照惯例需要对listen的fd进行关闭
        //所以一定要记住create的时候这个微妙的fd关闭——————因为后面的对于fd进行关闭，都是一关就关两个了
        close(listen_fd);   
    }

    //此时需要对epoll的内容进行添加了，所以此时就需要对进行添加的内容进行初始化的操作
    //所以这个初始化的时机和bind之前对具体绑定什么内容是一模一样的。
    //所以这两者都理解为要进行绑定之前必须要先进行初始化即可辅助记忆
    struct epoll_event ev;
    memset(&ev,0,sizeof(ev));   //所以这步的上下几步都是和bind的初始化完全相同，所以它俩放一起记是最好的
    
    ev.data.fd=listen_fd;   //这个暂时想不到好办法，暂时就无脑这样记吧。记不住也要反复记

    ev.events=EPOLLIN;   //这里就是最最核心的———————对于是读事件还是写事件进行添加

    ret=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,listen_fd,&ev)
    //第三个参数表达的是：(就比如这次是进行添加操作)进行添加监听的是listen_fd,所以本次想要进行操作的是哪个fd，就将哪个fd填到第三个参数中
    //因为在这个函数中第三个参数是最灵活的参数。所以后面进行操作的时候，基本上要改的就是第三个参数

    //这里的第四个参数就完全等价于bind的第二个参数——————即传入这个用于初始化的结构体

    /*
    注意对于老师会所的红黑树根据fd进行排序，指的是epoll_ctl的第三个参数(就比如代码中传入的listen_fd)
    所以这个第三个参数传入的是对于哪个fd进行监控，第一个参数传入的是具体存放到哪个epoll中进行监控
    所以这个ctl函数的四个参数还是非常非常细节的，第四个参数是和bind一样的如何进行初始化的结构体参数
    */
    if(ret<0){
        perror("epoll_ctl");
        close(listen_fd);
        close(epoll_fd);
        return EXIT_FAILURE;
    }
    //所以除了创建那一步是只close一个fd，后面的所有步骤都是一下close两个fd

    //定义一个结构体数组来拷贝就绪链表中都就绪了什么内容
    struct epoll_event ev_str[100]={0};   //数组的初始化直接写0即可,这点我都能忘了
    //这个要对比vector的几种初始化方式才可以——————即要能实现对于普通数组和动态数组初始化的明确辨析，才算把初始化记忆透彻了

    while(1){   
        //然后就是进入最核心的监控代码了——————select的代码也是这样和while(1)配合来实现轮询监控的

        int ready_num=epoll_wait(epoll_fd,ev_str,100,5000);
        //这里的第三个参数就是填准备用来拷贝过来的数组大小(一般都是像这里这样直接填数字)
        //第四个参数比较细节，填的是ms为单位的超时时间
        //并且第四个参数填-1，可以用来表示是永久阻塞直到有内容被监听就绪了(所以和select也是一模一样的原理)


        if(-1==ready_num&&(EAGAIN==errno||EINTR==errno)){
            //errno的作用：表示当前的错误码,用于和perror配合使用的
            //表示此时信号被影响了，这里也是大写的枚举值被写在左边，errno作为变量判断的实现
            //所以左边的都是errno的枚举值
            continue;
        }else if(-1==ready_num){


        }else if(0==ready_num){

        }else{    //到这里终于是有多少真正就绪的内容了(前面都是错误处理)
           for(size_t idx=0;idx<ready_num;idx++){
            if(ev_str[idx].data.fd==listen_fd){     
                //由于之前在进行初始化的时候就是使用.data.fd，所以这里也是使用的.data.fd
                //所以以后就没有所谓的.data了，全都是记为.data.fd即可。就像.sin_addr.s_addr一样
                //所以这两个结构体的共同点真的太多了，对比着看就能两个全都记下来

                //表示有新的链接到来,所以此时才需要进行accept操作
                //——————因此可以理解为epoll就是比原本的循环迭代法多了个epoll监听，其他的都是完全相同的思路

                //对于accept函数：
                //第一个参数：表示监听的总机fd———————所以没什么理由，这里accept就是用来分配分机的fd的，所以当然有啊传入总机的fd
                //第二个参数：表示用于接收新链接的客户端fd的指针
                //第三个参数：表示客户端地址的指针
                //第四个参数：表示客户端地址的指针的大小
                //返回值：表示新链接的fd

                /*
                所以这个第二个参数才是核心——————即上面的bind函数和这里的accept函数虽然参数完全相同，
                但是这里申请两个不同的fd，两个不同的结构体，核心原因就是accept的总机和分机之间的关系的这个重要细节所导致的
                所以这回总算搞懂为什么这两个函数形式如此相同，但是传入的具体内容不能相同了——————是从accept的总机分机细节来理解的
                */

                struct sockaddr_in cilentaddr;   //此时总算是具体的链接的结构体了
                memset(&cilentaddr,0,sizeof(cilentaddr));
                socklen_t cilentaddr_len=sizeof(cilentaddr);
                int net_fd=accept(listen_fd,(struct sockaddr*)&cilentaddr,&cilentaddr_len);
                //这里尤其要注意：这里有个特点就是传len的指针，而不是长度本身。这是accept最容易有的坑

                if(net_fd<0){
                    perror("accept");
                    
                    //close   根据我的分析，不能进行close操作，所以确实也是不能因为一个连接不成功就断开了监听
                    //所以accept是比较特殊的，不能对错误处理进行close的函数
                    
                }

                //然后对新建立的链接进行读事件的监听————————难怪会有下面的else，原来是因为这里的前人栽树后人乘凉了
                //所以这种前人栽树后人乘凉也是写代码的时候在while(1)这种代码中，非常非常常见的编写思路
                //尤其常见于while(1)和if-else相配合的问题中

                //这里是直接对于ev结构体进行复用即可————————所以意味着结构体也是可以复用的，不仅仅只有变量可以用来复用
                ev.data.fd=netfd;
                ev.events=EPOLLIN;
                epoll_ctl(epoll_fd,EPOLL_CTL_ADD,net_fd,&ev);
                //再次重申一遍：这里虽然ev结构体中已经有net_fd了，但是由于epoll_ctl是基于红黑树实现的，所以第三个参数传入的必要性是作为key用于有序排序使用
                //这个整个epoll是基于红黑树，然后ctl是用来维护红黑树结构的函数，这两点一定要极其极其熟练才可以应对面试


            }
            else{   //表示此时不是有新链接产生所导致的读操作，而是之前的链接产生了读操作
                //所以对于是读还是写，主体是谁？这两个问题会贯穿我后面的所有

                char buf[1024]={0};   //用于进行读数据的用户态缓冲区定义
                
                //对于新链接和对于旧链接的处理方法是不同的：对于新链接的处理方法是加入即可，旧链接触发必须要进行recv成果的产出
                ret=recv(ev_str[idx].data.fd,buf,sizeof(buf),0);
                //这个虽然是单纯的recv函数，但是这里的一个亮点(即和纯recv函数的不同点)
                //就是使用了数组下标+.data.fd这样的方式进行的获取。这种方法将是以后epoll的无脑通用写法，对这个写法一定一定要极其熟练

                if(ret==0){   //注意：对于recv，只要是=0就表示断开了，而不是<0才表示错误(对于send不同，send是==0表示断开了)
                    //所以我对于断开链接没写，难怪没印象了

                    /*
                    这个如果判断为是断开连接的内部代码逻辑极其极其重要
                    原因是如果在断开连接之后进行recv，就会反复发送
                    */

                }
                
                /*
                过了ret==0判断，此时就是正常传输了，所以此时就可以对recv的内容进行处理了
                对于recv的内容进行处理的范式是：先打印收到了多少字节的内容，再打印具体收到了什么内容
                */

                printf("recv %d bytes data:%s\n",ret,buf);

            }
           }

        }

    }   //end of 监听循环while(1)







    

    return 0;
}