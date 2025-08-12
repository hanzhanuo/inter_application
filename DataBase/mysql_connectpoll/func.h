#ifndef __FUNC_H
#define __FUNC_H


#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>  //mysql的客户端头文件
#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <stdexcept>

#include "logger.cpp"


#define DEBUG_LOG(msg) \
    std::cout << "[" << __FILE__   ":" << __LINE__ << "][" << __func__ << "] "  << msg << std::endl


#define SIZE(a) (sizeof(a)/sizeof(a[0]))

//进行最开始的参数个数检查的操作
#define ARGS_CHECK(argc, num)   {\
    if(argc != num){\
        fprintf(stderr, "ARGS ERROR!\n");\
        return -1;\
    }}

//进行错误判断的函数
#define ERROR_CHECK(ret, num, msg) {\
    if(ret == num) {\
        perror(msg);\
        return -1;\
    }\
}



#endif // __FUNC_H__