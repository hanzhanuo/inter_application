#include "func.h"
#include <asm-generic/socket.h>
#include <cstdlib>
#include <sys/socket.h>  //只要是使用网络编程，就也要包含这个库，所以这是我func.h的有一个疏漏
#include <sys/epoll.h>
#include <sys/select.h>   
//我光想着epoll需要头文件了，忘了select也需要同样的头文件了，所以这里的错误实在是太基础了。。。,重新迭代我的func.h 
#include <unistd.h> 
    


int main(int argc,char** argv){


    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        perror("socket");
        return EXIT_FAILURE;
    }

    //设置addr可以复用
    int on=1;   //on只有=1才是开启了，=0就是地址复用关闭了
    setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    //这里我原本写错了——————一定要记住是setsockopt，而不是setsocketopt

    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));   //每次结构体完了我都会忘记这个memset操作，对这个
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    //我这里对于等号右边又记错了——————等号右边一定是先inet，再addr，不要写反了
    server_addr.sin_port=htons(8080);
    /*
    在进行配置的时候一定是主机字节序转换为网络字节序；只有在进行展示的时候才是网络字节序转换为主机字节序
    并且只可能使用到s，不可能使用到long了
    */

    int ret=bind(listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr));
    /*
    所以bind函数的参数列表和memset的参数列表非常非常像，通过memset来助记想起来这个函数的参数应该怎么填
    所以整个网络编程就是一个大型的通过一个内容助记另一个内容的整体连贯下来的默写操作
    网络编程只要前期下功夫把所有的细节全部都搞懂了，那么后期都是直接默写的写法
    所以网络编程难点不是写，而是对于流程中的一些非常小的易错点的把握
    */
   if(ret<0){
    perror("bind");
    close(listen_fd);
    return EXIT_FAILURE;
   }

   ret=listen(listen_fd,128);    //listen函数只有链接队列的长度需要填，所以listen是个极其简单的函数，比其他的添加监听函数都简单的多
   if(ret<0){
    perror("listen");
    close(listen_fd);
    return EXIT_FAILURE;
   }


   fd_set ready_set,monitor_set;
   /*
   这表明创建了两个位图：
   一个是用来进行判断处理的，一个是用来对所有的监听进行备份的
   所有的监听添加操作都是对备份集合进行的，所有的select处理操作都是对就绪集合处理的 
   */

   FD_ZERO(&ready_set);
   FD_ZERO(&monitor_set);
   FD_SET(listen_fd,&monitor_set);  //这就是上面说的，所有的监听添加操作都是对于备份集合进行的操作
   
   int max_fd=listen_fd;
   int temp_fd=max_fd;   //我写这个完全是为了对于旧链接进行遍历的时候尽可能压缩遍历的个数，所以属于我对老师代码的一种优化吧   
   /*
   对于select的监控操作，每次都只监控到自认为是最大的操作位置中
   所以这也是种处理哲学——————每次对新添加的fd进行设置，
   更好的方法可以每次新添加的和当前的max_fd进行比较，从而更新最大值(所以这都是原本C语言中最最基础的操作的封装)
   所以当前设置listen_fd为当前最大，是因为此时只有listen_fd这一个fd，所以后面再添加的时候，才有必要比较是哪个fd最大
   */

   while(1){   //无论是对于epoll，还是对于select，都是需要写在while(1)内部的，这也应该是无脑记住的——————只要是多路复用并发，就写在while(1)里不断进行判断
    
    //每次都需要对ready先进行清零，再进行赋值操作。所以每次再进行一次清零，是因为又进入新的循环了
    //每次进行赋值前都需要清零来避免进行数据污染。这是应该无脑进行的操作
    FD_ZERO(&ready_set);
    ready_set=monitor_set;   //位图直接通过赋值运算符拷贝即可——————一定要记住它很简单


    int ready_num=select(max_fd,ready_set,NULL,NULL,NULL);
    //并且这里的select也是有就绪个数的(虽然这个就绪个数不会影响select必须对位图进行遍历，但是可以用来打印关键信息)
    //所以只要是存在的返回值，就都无脑打印出来即可，至少需要无脑进行的一步就是进行关键节点的打印
    printf("ready num is %d\n",ready_num);

    /*
    然后就是和epoll一样的，对于新链接和旧链接分开处理(新链接要建立链接，旧链接要产量)
    并且新链接能100%确定是对于listen_fd的监听所触发的，旧链接就没法确定具体是哪个链接，所以就只能对整个位图进行遍历
    所以这个由遍历所导致的if-else分离就是select相比于epoll的其中一个实现原理不同导致的代码结构的不同点
    */

    if(FD_ISSET(listen_fd,&ready_set)){   
        //所有的fd都是数字，所以不用纠结ready_set能不能看懂listen_fd是什么，最终编译器都会转换回数字的

        //进行建立新链接的操作
        //这里既要申请新的fd，又要申请新的结构体。这两个资源都是为accept所导致的接到分机准备的
        //所以这里的为分机准备完整资源，就是一个细节的知识点问题。这就是对于细节知识点进行深刻理解的意义
        struct sockaddr_in cilent_addr;  //为分机准备的，专门用来存储新接收的客户端的具体地址的
        //这个接收具体地址和epoll的epoll_event结构体是一模一样的原理，都是用来承接基本信息的
        socklen_t addr_len=sizeof(cilent_addr);
        int net_fd=accept(listen_fd,&cilent_addr,&addr_len);

        if(net_fd<0){   //这里就像socket直接创建实例一样，只要是fd，就都需要进行<0的错误判断
            perror("accept");
            /*
            然后好像不用进行关闭链接，直接继续进行下面的步骤即可
            所以这里甚至都不用写continue重新进行select，直接继续进行旧链接的判断即可
            这个错误判断是最有特色的错误判断，一定一定要熟练
            */
        }

        /*
        打印关键信息——————这里就需要对addr这个结构体的ip和端口号打印出来了，所以展示就是我上面说的网络字节序到主机字节序
        所以是把server和cilent的都打印出来，所以这里就是addr_intoa来实现对于ip的打印
        */

        FD_SET(net_fd,&monitor_set);
        if(net_fd>max_fd){
            //一下更新两个fd，所以每次都是对于上一次的max_fd成功存储的内容进行旧链接的位图遍历
            temp_fd=max_fd;
            max_fd=net_fd;   //对所有监听fd的最大值进行更新
        }
    
    }    //新链接处理结束，后面就是对所有旧链接进行处理了


    /*
    对旧链接进行处理——————总结：除了对于位图进行遍历以及少了else结构，其实我认为和epoll的从代码结构没有什么区别
    所以一定要勇敢记住没什么区别，然后多写几遍，就能熟练掌握select写法了
    */
    char recv_buf[1024]={0};
    for(size_t idx=0;idx<temp_fd;idx++){
        //if(FD_ISSET(i, &readset) && i != listenfd)
        if(FD_ISSET(idx,&ready_fd)){   
            /*
            由于我做的temp_fd的优化，所以这里甚至都不需要进行老师的listenfd判断了
            老师这种写法非常像聊天室问题的"自复制问题"，所以要对这种思想非常熟练
            以后在进行网络相关的转发和判断的时候，这种自复制判断会是非常非常关键的代码健壮性问题
            */
           //学习上面的对于每次进来的ready_set进行清空避免污染，这里的缓冲区数组也每次进来后都清空避免污染
            recv_buf[1024]={0};
            int ret=recv(idx,&recv_buf,sizeof(recv_buf),0);
            
            if(ret=0){   //对端断开链接了


                FD_CLR(idx,&monitor_set);   //我竟然能把这步最重要的第一步给忘了，我真是罪该万死
                close(idx);
                printf("connect %d is close\n",idx);
                //close(idx);    这里我写错顺序了，应该这端也先close了然后再打印，实在太蠢了
                continue;
                /*
                问题：为什么上面的新链接不需要continue，这里需要continue？
                原因是每次的listen产生的新链接一定是有且仅有一个的，所以不需要进行continue，否则就会回到最外层的while(1)循环的开头
                这里由于是对旧链接进行遍历，所以是一个while(1)内部的新循环，所以是可以放心continue到这个内部循环的开始的
                */

                //原本上面都是三步走，如果加了对于客户端存储的数据结构的话，就需要多添加一步对于这个断开的客户端进行删除
                //所以这就是为什么以前一直都是四步走，现在总是三步走，就是因为没有对客户端进行存储
            }

            //不是对端关闭了的情况，此时就可以聊天室转发了
            for(size_t idx=0;idx<list;idx++){
                if(){
                    send(idx;&,);
                    //在进行发送的时候，一定一定要使用strlen，只有在recv时候不确定接收多少内容的时候，才需要使用sizeof(buf)
                    //这个细节非常重要，我却因为写的太少忘了
                }
            }

        }
    }
   }
   


    return 0;
}