#include "func.h"
#include <asm-generic/socket.h>
#include <cstdlib>
#include <sys/epoll.h>

#define MAX_EVENT_SIZE 100
#define EPOLL_TIMEOUT 5000

struct UserInfo
{
    int sockfd;
    char name[20];
};


int main()
{
    //1. 创建监听的套接字
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

    //2. 绑定网络地址
    int ret = bind(listenfd, (const struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if(ret < 0) {
        perror("bind");
        close(listenfd);
        return EXIT_FAILURE;
    }

    //3. 监听新连接的到来
    ret = listen(listenfd, 12800);
    if(ret < 0) {
        perror("listen");
        close(listenfd);
        return EXIT_FAILURE;
    }

    //4. 创建epoll的实例
    int epfd = epoll_create1(0);
    if(epfd < 0) {
        perror("epoll_create1");
        close(listenfd);
        return EXIT_FAILURE;
    }
    printf(">> epfd: %d\n", epfd);

    /* 对于union中可以传入的void*的进一步传入struct类型的指针的写法：
    struct UserInfo userInfo; 
    userInfo.sockfd = listenfd; 
    strcpy(userInfo.name, "wangdao"); 

    所以以后只要看到void*(在linux编程中，好像非常常见使用到void*)
    就最最立刻要想到定义自定义类型，然后把自定义类型传入进来
    所以以后要非常熟练的记住这种肌肉反应
    */

    //5.epoll添加对listenfd事件的监听
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = listenfd;
    //ev.data.ptr = &userInfo;   
    //上面就是对于union的void*字段传入自定义结构体类型的方法(但是要注意传入的是指针)
    ev.events = EPOLLIN;//对listenfd上的读事件进行监听
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    if(ret < 0) {
        perror("epoll_ctl");
        close(listenfd);
        close(epfd);
        return EXIT_FAILURE;
    }

    struct epoll_event evtArr[MAX_EVENT_SIZE] = {0};
    //6.事件循环，不断的对事件进行监听
    while(1) {
        printf("before epoll_wait\n");
        //int nready = epoll_wait(epfd, evtArr, MAX_EVENT_SIZE, EPOLL_TIMEOUT);
        int nready = epoll_wait(epfd, evtArr, MAX_EVENT_SIZE, -1);
        if(nready == -1 && errno == EINTR) {
            continue;//被信号影响
        } else if(nready == -1) {
            perror("epoll_wait");
        } else if(0 == nready) {
            printf("epoll timeout.\n");
        } else {
            //当有多个客户端同时连接进来时(排除已经建立好的连接发数据)，
            //新连接的处理都是由listenfd来进行的，listenfd只有一个
            //所以epoll_wait的返回值都是1
            printf(">> nready: %d\n", nready);
            //nready大于0的情况，有文件描述符就绪
            for(int i = 0; i < nready; ++i) {
                int fd = evtArr[i].data.fd;//遍历数组获取就绪的文件描述符
                /* struct UserInfo * pret = (struct UserInfo*)evtArr[i].data.ptr; */
                /* int fd = pret->sockfd; */
                
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
                    ev.events = EPOLLIN | EPOLLET;//采用边缘触发
                    //所以这里进行的是每个放入epoll监听的事件，都需要分别采取边缘触发的方式
                    //所以设置边缘触发的单位是在每个事件添加的时候进行设置的，所以是种很细粒度的操作
                    epoll_ctl(epfd, EPOLL_CTL_ADD, netfd, &ev);
                } else {
                    //netfd已经建立好的连接上有读事件就绪
                    char buff[128] = {0};
                    printf("proess recv operation\n");
                    //6.4 查看内核缓冲区中的数据量
                    ret = recv(fd, buff, sizeof(buff), MSG_PEEK);
                    if(0 == ret) {
                        //6.6 表示连接已经断开了
                        ev.data.fd = fd;//只需要指定文件描述符,不需要关心事件
                                        //因为是做删除操作
                        //从epoll监听的红黑树上删除
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
                        close(fd);
                        printf("conn %d has closed.\n", fd);
                        //注意：这里的打印对于哪个fd已经关闭了，需要注意fd的序号:因为fd的012是三大标准
                        continue;
                    }


                    //当内核接收缓冲区数据超过20个字节时才进行处理
                    //这里是对于前面的recv进行peek以及每个事件都设置为边缘触发只提醒一次
                    printf("contents %d bytes.\n", ret);
                    if(ret >= 20) {
                        //从内核接收缓冲区中移走数据
                        ret = recv(fd, buff, sizeof(buff), 0);
                        //6.5 连接正常情况下
                        printf("recv %d bytes, content: %s\n", ret, buff);
                        //服务器回显客户端的信息
                        ret = send(fd, buff, strlen(buff), 0);
                        printf("send %d bytes.\n", ret);
                    }
                    
                }
            }
        }
    }

    close(epfd);
    close(listenfd);
    return 0;
}

