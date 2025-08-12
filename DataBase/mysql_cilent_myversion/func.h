


#include <stdio.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>  //mysql的客户端头文件
#include <string>
#include <string.h>
#include <vector>
#include <iostream>


#define DEBUG_LOG(msg) \
    std::cout << "[" << __FILE__ << ":" << __LINE__ << "][" << __func__ << "] " << msg << std::endl