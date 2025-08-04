#include "func.h"
#include <asm-generic/socket.h>
#include <cstdlib>
#include <sys/epoll.h>


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

    ev.events=EPOLLIN|EPOLLET;   //这里就是最最核心的———————对于是读事件还是写事件进行添加

    ret=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,listen_fd,&ev);
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
            //这种返回值表示epoll_wait出错了。只要是ret=-1，都100%是出错了，不可能是别的情况
            //对于这个函数出错了没必要进行close操作，因为这个函数出错了，所以当前监听的net_fd已经没有意义了
            //一定要记住此时是fd没有意义了所导致的，之前我一直都理解错了
            perror("epoll_wait");
            break;
        }else if(0==ready_num){
            //perror("")    注意这里不是错误，如果接收不到就代表是超时了，但是不是错误
            //所以意味着perror的使用非常严格——————只有在ret=-1的时候才使用perror，其他情况想要实现交互就都使用printf来实现
            printf("timeout\n");
            //并且当使用上printf之后，就一定要无脑写上换行符，这个是必须肌肉记忆的操作
            
        }else{    //到这里终于是有多少真正就绪的内容了(前面都是错误处理)
           for(size_t idx=0;idx<ready_num;idx++){
            if(ev_str[idx].data.fd==listen_fd){     
                

                struct sockaddr_in cilentaddr;   //此时总算是具体的链接的结构体了
                memset(&cilentaddr,0,sizeof(cilentaddr));
                socklen_t cilentaddr_len=sizeof(cilentaddr);
                int net_fd=accept(listen_fd,(struct sockaddr*)&cilentaddr,&cilentaddr_len);
                //这里尤其要注意：这里有个特点就是传len的指针，而不是长度本身。这是accept最容易有的坑

                if(net_fd<0){
                    perror("accept");
                    
                }

                //这里是直接对于ev结构体进行复用即可————————所以意味着结构体也是可以复用的，不仅仅只有变量可以用来复用
                //这里的结构体复用，以及每个监听的内容全局范围值只需要添加一次就可以了。
                //上面这两个操作组合起来，就是epoll相比于select来说，最重要的优点
                ev.data.fd=net_fd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epoll_fd,EPOLL_CTL_ADD,net_fd,&ev);
                

            }
            else{   //表示此时不是有新链接产生所导致的读操作，而是之前的链接产生了读操作
                //所以对于是读还是写，主体是谁？这两个问题会贯穿我后面的所有

                char buf[1024]={0};   //用于进行读数据的用户态缓冲区定义

                printf("process recv operation\n");
                //这种对于关键节点的打印，就可以理解为是对于关键节点的打印调试操作
                
               /*
               此时对于旧链接的成果产出，一定一定要用MSGPEEK来配合边缘触发，否则recv仍然会被读出来
               这里的目的就是实现只提醒一次并且不会被读出来，所以每次添加写边缘触发了，成果产出也一定要写peek操作
               */

                ret=recv(ev_str[idx].data.fd,buf,sizeof(buf),MSG_PEEK);
                //就是使用了数组下标+.data.fd这样的方式进行的获取。这种方法将是以后epoll的无脑通用写法，对这个写法一定一定要极其熟练
                //或者其实还有一种写法：在每次遍历之初就把它用一个fd来承接，后面写起来就比较方便

                if(ret==0){   //注意：对于recv，只要是=0就表示断开了，而不是<0才表示错误(对于send不同，send是==0表示断开了)
                    //recv=0表示的是对端断开了——————一定要对这个有立刻反应的肌肉记忆
                    
                    //由于这里使用的是边缘触发，即使是断开后缓冲区中仍有内容也不会反反复复提醒了，只会提醒一次
                    //但是对于已经断开了之后的处理方式是不变了————————即这种处理方法并不会因为是什么提醒方式而改变

                    //对于优雅关闭的四步走：删除监视，关闭链接，打印关闭事件，continue
                    //这四步走我还是经常会忘，所以一定要多练
                    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,ev_str[idx].data.fd,&ev);
                    close(ev_str[idx].data.fd);
                    printf("close connect %d\n",ev_str[idx].data.fd);
                    continue;

                }


                if(ret>20){   
                    /*
                    所以ret>20这里告诉了我一个更深入的理解:
                    这个recv的返回值本质上是缓冲区还有多少内容可读，这就是为什么链接断开了但是recv还有内容的时候，就会反复进行提醒
                    所以这里的ret>20表示的是缓冲区中的内容已经积累超过20字节
                    */

                    //之所以用来承接的用户态缓冲区buffer需要定义在前面，是因为所有的recv函数都需要这个buffer作为参数，而非仅仅在自己真正想读取的时候才试用这个作为参数
                    ret=recv(ev_str[idx].data.fd,buf,sizeof(buf),0);
                    //此时才能放心进行最后一个参数是0的recv操作
                    printf("recv %d bytes data:%s\n",ret,buf);

                    //之前都是直接进行对于客户端的send的回显操作
                    for(int idx=0;idx<list_len;idx++){
                       if(){  //如果是链接没有断开并且此时不是发送端自己
                        send();   //通过遍历把所有的内容发送给剩余的所有客户端
                       } 
                    }

                }
                
                

               

            }
           }

        }

    }   //end of 监听循环while(1)



    return 0;
}