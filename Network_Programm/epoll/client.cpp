#include <cstdlib>
#include <func.h>

int main()
{
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8000);
    //serveraddr.sin_addr.s_addr = inet_addr("192.168.30.129");
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");//代表本机地址,所有的本地环回地址使用的都是127.0.0.1
    //所以在进行网络编程的时候，当你需要连接的是本地的服务端的时候，就直接使用127.0.0.1即可
    //当你需要连接的是远程的服务端的时候，就需要使用远程的IP地址
    //所以这里使用本地环回地址完全是因为此时的服务器在本地电脑上;如果是做项目的时候的天明的那个服务器，就必须要填特定的远程地址了

    int ret = connect(clientfd, (const struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if(ret < 0) {
        perror("connect");
        close(clientfd);
        return EXIT_FAILURE;
    }

    printf("connect success\n");


    sleep(1);
    const char * msg = "hello, server";
    ret = send(clientfd, msg, strlen(msg), 0);
    printf("send %d bytes.\n", ret);
    //基本上可以说，所有的返回值都需要进行错误判断(其中包括这里的对于recv，也是必须要进行错误判断的)
    //但是唯独这个send，由于是对外发送而不是对内承接，所以只需要打印出发送了多少内容即可，不需要判断小于0的情况
    //但是这里确实有ret<0就表示发送失败的情况这么一说，所以进行判断倒也挺好的，有助于自己在写所有函数时思想上的统一
    if(ret<0){
        perror("send");
        // close(clientfd);
        // return EXIT_FAILURE;
        //但是发送失败有没有必要重新发呢，我感觉这里可以发送失败重新发送，这样写起来更能体现发送的多次尝试的过程
    }

    //指定用户态接收缓冲区
    char buff[1024] = {0};
    //当第三个参数对数组使用sizeof运算符时，
    //表示最大可以接收数组的空间的数据量
    ret = recv(clientfd, buff, sizeof(buff), 0);
    printf("recv %d bytes.\n", ret);
    printf("content: %s\n", buff);

    //断开连接
    close(clientfd);
    return 0;
}

