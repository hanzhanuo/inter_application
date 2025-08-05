#include "InetAddress.hpp"


namespace apion{

    InetAddress::InetAddress(unsigned short port, const std::string & ip)
    {
        /*
        借助数据成员实现对于ip和端口的配置操作
        切记这个类是用来进行配置操作的，而不是用来进行读取ip和端口操作的
        (即这个类是用来实现写操作的，而不是用来实现读操作的)
        */

        memset(&m_addr,0,sizeof(m_addr));
        m_addr.sin_family=AF_INET;
        m_addr.sin_port=htons(port);
        m_addr.sin_addr.s_addr=inet_addr(ip.c_str());
        /*
        每次最后这个我都背不过应该怎么写。这个一定一定要背熟
        并且这个有亮点——————即这个让我知道了以前一直使用的都是char*数组，所以这里使用字符串的时候，必须要进行字符串转换操作
        */

    }

    /*
    上面的构造函数中说了：上面是机型写操作即配置操作的;下面这两个函数就是进行读操作getter操作的
    即只要获取了这个addr_in的struct，就可以通过这两个成员函数获取这两个数据
    这就是为什么ip和端口不是数据成员——————因为只需要这一个结构体数据成员，剩下的所有数据成员都就自然getter了
    所以严格来说，这两个函数本质上就是用来获取这个数据成员结构体中的成员的，所以其本质上也是一个get数据成员的方法，所以我仍然将其视作get函数
    */

    std::string InetAddress::get_ip() const{
        return inet_ntoa(m_addr.sin_addr);
    }
    unsigned short InetAddress::get_port() const{
        return ntohs(m_addr.sin_port);
    }


    //用来反向获取的构造函数(所以反向获取要放在这些)
    InetAddress::InetAddress(const struct sockaddr_in& addr)
        :m_addr(addr){

    }


}

