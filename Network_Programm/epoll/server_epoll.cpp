#include <asm-generic/socket.h>
#include <cstdlib>
#include <func.h>
#include <sys/epoll.h>

#define MAX_EVENT_SIZE 100
#define EPOLL_TIMEOUT 5000

int main()
{
    /*
    1. 创建监听的套接字
    注意：这里之所以需要创建监听套接字，只是因为把名字定义成监听套接字，用于表示它是用于监听功能的
    但是它仅仅是名字不一样，用于体现它是用来监听的fd，但是它本质上其实是服务器端fd，所以叫serverfd也是完全一样的
    不过它之所以不叫server_fd,是因为在accept函数的细节讲解时已经讲解过了：
    即它只是一个总机fd，在accept之后，会对每个客户端发来的链接分配单独的不同的net_fd,用于和每个客户端独立联系
    所以上面这些叽里咕噜一大堆，才是不使用server_fd的原因
    (其实我认为核心原因就是它只是用于监听的总机，不是用于和客户端进行通信的分机，所以它是专门用于listen的)    
    */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }
    printf(">> listenfd: %d\n", listenfd);


    //设置套接字属性:网络地址可以重用
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8000);
    //serveraddr.sin_addr.s_addr = inet_addr("192.168.30.129");
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");//代表本机地址
    //注意写这步的时候要时刻小心——————小心s_addr经常会漏写。

    //2. 绑定网络地址
    int ret = bind(listenfd, (const struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if(ret < 0) {
        perror("bind");
        close(listenfd);
        return EXIT_FAILURE;
    }

    //3. 监听新连接的到来
    ret = listen(listenfd, 1);
    if(ret < 0) {
        perror("listen");
        close(listenfd);
        return EXIT_FAILURE;
    }

    //4. 创建epoll的实例——————即创建epoll的fd，然后后面所有epoll操作都是通过这个fd进行的
    int epfd = epoll_create1(0);
    if(epfd < 0) {
        perror("epoll_create1");
        close(listenfd);
        return EXIT_FAILURE;
    }
    printf(">> epfd: %d\n", epfd);

    /*
    5.epoll添加对listenfd事件的监听
    所以添加监听和进行监听是两个完全不同的函数
    这个也是完全可以和select函数的监听添加函数FD_SET函数进行对比，就能发现两者也是完全一模一样的
    */
    
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;//对listenfd上的读事件进行监听
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    if(ret < 0) {
        perror("epoll_ctl");
        close(listenfd);

        close(epfd);   //所以这告诉了我一个细节处理的思想：即只要是出现了错误，就要把所有的fd都进行关闭处理操作
        return EXIT_FAILURE;
    }

    //用来拷贝就绪队列的数组(这个数组是为epoll_wait准备的)
    struct epoll_event evtArr[MAX_EVENT_SIZE] = {0};
    
    //6.事件循环，不断的对事件进行监听
    while(1) {
        int nready = epoll_wait(epfd, evtArr, MAX_EVENT_SIZE, EPOLL_TIMEOUT);
        
        /*
        对于epoll_wait返回值的拆分判断：
        -1: 表示epoll_wait被信号中断了
        0: 表示超时了
        大于0: 表示有文件描述符就绪
        */
        if(nready == -1 && errno == EINTR) {   //INTR表示interrupt的含义
            continue;//被信号影响
        } else if(nready == -1) {
            perror("epoll_wait");
            //这里表示epoll_wait函数错误了，但是并没有关闭任何
            //所以这个和accept一模一样，都是如果超时无所谓，因为还没进行创建链接，并且这个错误是孤立的，不会导致整个网络的监控断开
            //所以只要是在这个while(1)循环内部实现的，应该都是孤立的内容，并且也一定是尝试链接的内容
            //所以孤立尝试链接的内容不会影响到整体，也因为未连接所以不会影响到其本身。所以才不需要close任何
        } else if(0 == nready) {
            printf("epoll timeout.\n");
        }
         else {   
            /*
            所以这个是对于epoll_wait返回值大于0的情况，即表示正常情况：有文件描述符就绪了
            所以接下来的分析就和select的有正常返回值的问题一样了
            因为大于0的时候，nready是多少，就代表这一轮epoll是有多少个文件描述符就绪了，所以才使用nready
            所以这里和select一模一样——————都是在每次监视器满足条件之后，进行完这一轮epoll监视器的监控，然后把这一轮满足条件的事件触发给进行区分处理(区分为是还没链接的写事件请求和已经链接的写事件请求)
            并且这种方式看来和Qt的事件系统也是一模一样的——————即虽然看起来event函数是每次只匹配了其中一种if分支就return true了，但是架不住event是循环往复进行的，所以就无所谓每次只处理单一的情况了
            所以三者都是while(1)的循环调用，并且这里下一步就是继续进行对于读事件是已经连接的还是未链接的这两种情况的分类讨论
            */
            printf(">> nready: %d\n", nready);
            //nready大于0的情况，有文件描述符就绪
            for(int i = 0; i < nready; ++i) {
                int fd = evtArr[i].data.fd;//遍历数组获取就绪的文件描述符
                
                if(fd == listenfd) {//listenfd的值为3
                    //6.1 有新连接过来
                    struct sockaddr_in clientaddr;
                    socklen_t len = sizeof(clientaddr);
                    int netfd = accept(listenfd, (struct sockaddr*)&clientaddr, &len);
                    if(netfd < 0) {
                        perror("accept");
                    }

                    //6.2 新连接信息的打印
                    printf("conn %d tcp %s:%d-->%s:%d has connected.\n",
                           netfd, 
                           inet_ntoa(serveraddr.sin_addr),
                           ntohs(serveraddr.sin_port),
                           inet_ntoa(clientaddr.sin_addr),
                           ntohs(clientaddr.sin_port));
                    //6.3 epoll对netfd进行读事件的监听
                    ev.data.fd = netfd;
                    ev.events = EPOLLIN;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, netfd, &ev);
                } else {
                    //netfd已经建立好的连接上有读事件就绪
                    char buff[128] = {0};
                    //6.4 接收数据
                    ret = recv(fd, buff, sizeof(buff), 0);
                    if(0 == ret) {
                        //6.6 表示连接已经断开了
                        ev.data.fd = fd;//只需要指定文件描述符,不需要关心事件
                                        //因为是做删除操作
                        //从epoll监听的红黑树上删除
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
                        close(fd);
                        printf("conn %d has closed.\n", fd);
                        continue;
                    }
                    //6.5 连接正常情况下
                    printf("recv %d bytes, content: %s\n", ret, buff);
                    //服务器回显客户端的信息
                    ret = send(fd, buff, strlen(buff), 0);
                    printf("send %d bytes.\n", ret);
                    
                }
            }
        }
    }    //end of while(1)

    close(epfd);
    close(listenfd);
    return 0;
}

