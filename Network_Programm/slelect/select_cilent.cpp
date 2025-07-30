#include "func.h"
#include <asm-generic/socket.h>
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>   //专门在unix下使用的一些库函数

int main(int argc,char** argv){

    //这里和epoll的理解一样，都是创建一个进行网络通信的fd的实体
    int cilent_fd=socket(AF_INET,SOCK_STREAM,0);   //重复习，这里的0表示默认使用第二个参数的协议



    //切记：客户端不需要绑定ip+端口，只需要对这个ip+端口进行请求即可。所以只有服务器端有bind和listen两个麻烦的操作，客户端写起来应该是非常非常方便的


    //助记：客户端只需要监控是标准输入和recv发过来的消息即可，其他的都不需要监控
    //所以客户端不需要海量监视，少量就绪，所以所有所有的客户端都无脑写select的方法即可
}