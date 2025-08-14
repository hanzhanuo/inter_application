#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__


#include <string>
#include <cstdint>

using std::string;

namespace wd
{
//枚举类型，相比宏可以做类型检查
enum TaskType
{
    TASK_TYPE_LOGIN_SECTION1 = 1,
    TASK_TYPE_LOGIN_SECTION1_RESP_OK,
    TASK_TYPE_LOGIN_SECTION1_RESP_ERROR,
    TASK_TYPE_LOGIN_SECTION2,
    TASK_TYPE_LOGIN_SECTION2_RESP_OK,
    TASK_TYPE_LOGIN_SECTION2_RESP_ERROR,
    
    TASK_TYPE_REGISTER1,
    TASK_TYPE_REGISTER1_RESP_OK,
    TASK_TYPE_REGISTER1_RESP_ERROR,
    TASK_TYPE_REGISTER2,
    TASK_TYPE_REGISTER2_RESP_OK,
    TASK_TYPE_REGISTER2_RESP_ERROR,

    TASK_TYPE_COMMON_MESSAGE
};

struct TLV
{
    uint32_t type;
    uint32_t length;
    char data[1024];
};

struct Packet
{
    uint32_t type;
    uint32_t length;
    string msg;
};

}//end of namespace wd








#endif