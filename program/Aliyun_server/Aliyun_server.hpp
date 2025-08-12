
/*
业务逻辑分析：
1. 该类主要负责与阿里云服务器进行交互，包括连接、断开、发送和接收数据等功能。
2. 需要考虑网络延迟、数据安全性等问题。
3. 可能需要实现重试机制，以应对临时网络故障。

写一个和上一个项目一样的服务器端进行监听，然后使用epoll进行监听
*/


#include "Socket.hpp"
#include "InetAddress.hpp"
#include "Acceptor.hpp"

namespace ghz{

class Aliyun_server{
    private:
        Acceptor _acceptor;
    public:
        Aliyun_server(unsigned short port, const string & ip = "0.0.0.0")
            : _acceptor(port, ip) {}

        void ready() {
            _acceptor.ready();
        }

        int accept() {
            return _acceptor.accept();
            //这里return的是对连接成功返回一个net_fd
        }

        int fd() const {
            return _acceptor.fd();
            //在这里实现了对于当前这个阿里云服务器的fd的获取
        }

        //所以上面这两个操作都是进行fd的获取的操作，然后获取了fd就交给阿里云服务器的epoll进行监听
        //阿里云服务器和我自己的服务器的区别：阿里云服务器不需要进行process的复杂计算，其余的所有操作应该都是和普通server一模一样的
        void setReuseAddr(bool on) {
            _acceptor.setReuseAddr(on);
        }
    };

}


    //设计一个启动链接的行为(就是TCPconnection封装的ready行为)


    //设计epoll监听的行为

    //是不是可以直接使用eventloop类呢？
    //或者进一步说：这个阿里云的ready是否可以直接使用acceptor类的操作进行转发调用呢？

    //直接使用accept类的操作来转发调用实现
