#include <func.h>

#include <map>
#include <set>
#include <sys/epoll.h>

#define MAX_EVENT_SIZE 100
#define EPOLL_TIMEOUT 1000

using std::map;
using std::set;

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

    //5.epoll添加对listenfd事件的监听
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.fd = listenfd;
    ev.events = EPOLLIN;//对listenfd上的读事件进行监听
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    if(ret < 0) {
        perror("epoll_ctl");
        close(listenfd);
        close(epfd);
        return EXIT_FAILURE;
    }

    //保持已经建立好的连接的fd和上一次发送数据的时间
    map<int, int> conns;

    struct epoll_event evtArr[MAX_EVENT_SIZE] = {0};
    //6.事件循环，不断的对事件进行监听
    while(1) {
        printf("\nbefore epoll_wait\n");
        //epoll会设置1秒超时
        int nready = epoll_wait(epfd, evtArr, MAX_EVENT_SIZE, EPOLL_TIMEOUT);
        //方案三: ...超时断开连接(不太适合，因为数据过来了，还没进行处理就直接踢出去)
        if(nready == -1 && errno == EINTR) {
            continue;//被信号影响
        } else if(nready == -1) {
            perror("epoll_wait");
        } else if(0 == nready) {
            printf("epoll timeout.\n");
            //方案四: ...超时断开连接 (不太适合)
            //在1秒之内没有任何文件描述符就绪时，才会进入该分支
            //但如果有fd就绪，就不会进入该分支
        } else {
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
                    ev.events = EPOLLIN;//默认情况下采用水平触发
                    epoll_ctl(epfd, EPOLL_CTL_ADD, netfd, &ev);
                    // 添加到conns中去
                    int initTime = time(nullptr);
                    conns[netfd] = initTime;
                } else {
                    //netfd已经建立好的连接上有读事件就绪
                    char buff[128] = {0};
                    printf("proess recv operation\n");
                    //6.4 从内核缓冲区中获取数据
                    ret = recv(fd, buff, sizeof(buff), 0);
                    if(0 == ret) {
                        //6.6 表示连接已经断开了
                        ev.data.fd = fd;//只需要指定文件描述符,不需要关心事件
                                        //因为是做删除操作
                        //从epoll监听的红黑树上删除
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);

                        //从conns中删除该连接
                        conns.erase(fd);
                        close(fd);
                        printf("conn %d has closed.\n", fd);
                        continue;
                    }
                    //查看消息
                    printf("recv from %d: %s\n", fd, buff);
                    //更新时间
                    conns[fd] = time(nullptr);
                    //执行转发操作
                    for(auto & elem : conns) {
                        //发送数据的客户端fd不需要接收自己的数据
                        if(elem.first != fd) {
                            send(elem.first, buff, strlen(buff), 0);
                        }
                    }
                    //方案一: ...超时断开连接(只处理了一个就绪的文件描述符，就开始断开连接)
                    //不太合适
                }
            }   //end of for
        }//end of else
        //方案二: ...超时断开连接
        //在将已经就绪的文件描述符全部处理完毕之后，再进行超时断开连接
        ////1秒超时之后，也会执行到
        int curTime = time(nullptr);
        set<int> pendingDeletingFds;
        for(auto & elem : conns) {//注意：在遍历一个容器的过程中，不适合直接删除元素
            if(curTime - elem.second >= 30) {
                close(elem.first);
                //conns.erase(elem.fisrt);//注意：没有直接删除，防止出现迭代器失效的情况
                pendingDeletingFds.insert(elem.first);
            }
        }
        //先保存下来，再统一删除
        for(auto & fd : pendingDeletingFds) {
            conns.erase(fd);
        }
        printf("\naftrer process timeout fds.\n");
    }//end of while(1)

    close(epfd);
    close(listenfd);
    return 0;
}

