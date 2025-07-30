#include "Task.hpp"

namespace ghz{

    //void Task::run override{}
    //错误的!! 我总算知道上次override为什么报错了
    //报错原因就是我头文件和实现文件左右互搏了
    //应该是对于Task的继承类才需要使用override，这里只是头文件和实现文件，所以这样写自然是大错特错
}