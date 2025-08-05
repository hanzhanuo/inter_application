#ifndef INETADRESS_H
#define INETADRESS_H

#include "func.h"
#include <string>

namespace apion{

class InetAddress {
   private:
     struct sockaddr_in m_addr;


   public:
    explicit InetAddress(unsigned short port,const std::string& ip="0.0.0.0");
    /*
    这里纯是我自己不熟练——————即正常来说：第一是必须要写构造函数，第二是和类名相同的一定是构造函数
    这两点都满足了但是我一开始却没看出来这是构造含糊

    第二：上面这个构造函数之所以可以设计为是explicit的，是因为
     */


     struct sockaddr_in* get_InetAddr_ptr() {
    return &m_addr;
    //有一次忘记了取地址，所以自己写的时候就是非常容易忘东忘西的
   }

    std::string get_ip() const;
    unsigned short get_port() const;
    //注意：这里的两个get函数并不是给外界获取数据成员的，而是让自己知道这个是需要操作才能获取到的

  

   //对于这种进行反向获取的构造函数，为了方便助记，我就直接放在所有的正向获取的getter函数后面了，而不是按顺序放在构造函数位置了
   explicit InetAddress(const struct sockaddr_in&);
    /*
    这个构造函数设计是getter函数的反向设计——————即getter函数是通过创建对象获取数据成员;
    这个构造函数的设计是为了 借助传参来return一个对象，
    (因为之前在讲基于对象的线程池的时候就说过:直接调用构造函数，就会生成一个临时对象。这个临时对象用于return绰绰有余了)
     */

};

}

#endif //INETADRESS_H
