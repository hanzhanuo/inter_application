#ifndef __FUNC_H
#define __FUNC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <error.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/uio.h>

#define SIZE(a) (sizeof(a)/sizeof(a[0]))
//using需要加分号，宏定义不需要加分号，但是切记宏定义要每个位置都加小括号，否则就可能出现问题

typedef void (*sighandler_t)(int);



//这下对于这种通用头文件中需要放什么就有参照了——————老师写的这个就是头文件的标杆

//这种通过加\的方式实现对宏函数的换行操作，C语言的内容一个都不应该忘
#define ARGS_CHECK(argc, num)   {\
    if(argc != num){\
        fprintf(stderr, "ARGS ERROR!\n");\
        return -1;\
    }}

#define ERROR_CHECK(ret, num, msg) {\
    if(ret == num) {\
        perror(msg);\
        return -1;\
    }}


#endif  //__FUNC_H
