#include "EventLoop.hpp"

namespace apion{

    EventLoop::EventLoop(Acceptor& acceptor)
    :m_epoll_fd(epoll_create())    
    /*
    这种在初始化列表中，使用成员函数来初始化数据成员的方法,我还真是从来没见过
    这种方法一定一定要会，实在是种非常流氓的先上车后补票的设计方法
    这种设计方法能极大的拓展我设计初始化列表的时候的设计思维边界
    */
    ,m_acceptor(acceptor)
    ,m_is_looping(false)
    ,m_evtArr(1000)
    //对于map由于没有需求，所以就不需要通过初始化列表进行初始化
    
    /*
    这种在初始化列表中初始化容器的方法我也是从来没用过
    前面总结了：容器是以什么形式进行初始化的，是由这个容器的使用需求决定的
    不过在我看来，对于容器的初始化这种复杂的场景，还是最好在构造函数的函数体中实现是最优的写法
    这里不过是因为确实只需要size，所以才使用这种方法进行初始化的
    */
    {
        /*
        int listen_fd=m_acceptor.get_fd();  
        注意:这里获取的是acceptor的fd，而不是listen_fd
        所以这里是对于accept本身进行监听，而不是对于产生的listen进行监听
        */
        epoll_addReadEvent(m_acceptor.get_fd());
    }

    void EventLoop::setAllCallBacks(TCPConnectionCallBack&& cb1,TCPConnectionCallBack&& cb2,TCPConnectionCallBack&& cb3){
        m_doConnection=std::move(cb1);
        m_doInfo=std::move(cb2);
        m_doClose=std::move(cb3);
    }


    void EventLoop::epoll_fdwait(){
        int ready_num=epoll_wait(m_epoll_fd,m_evtArr.data(),m_evtArr.size(),5000);   
        /*
        一般都统一设计为5s超时是最好的写法
        我还有问题：这里的evtArr传入的不应该是首地址和数组大小吗？这里这个第二第三个参数传入的到底是什么？
        哦原来vector的data()函数返回的是vector的首地址啊，这个概念我完全忘了
        */
        if(ready_num==-1&&errno==EINTR){
            return;   //表示此时是被中断信号干扰的所导致的
        }else if(ready_num==-1){
            perror("epoll_wait");   //-1一般都是直接返回当前函数的函数名
        }else if(ready_num==0){
            perror("epoll_timeout");
        }else{
            //这回总算是正常的接收情况了
            for(size_t idx=0;idx<ready_num;idx++){
                int fd=evtArr.
            }
        }
        
    }


    void EventLoop::handle_NewConn(){
        
        //if()
        /*
        都不需要进行判断，判断是留给测试文件处理的，这里只需要进行对已经确定了是新链接了的话，需要做什么进行封装即可
        所以这就是进行封装的思想————————即应当是把所有分类讨论内部的行为进行封装，而不是也囊括分类讨论
        以后在自己进行行为封装的时候，就一定要能熟练记住:只需要封装如何进行处理，不需要封装是如何进行判断的
        */

        int net_fd=m_acceptor.accept();   
        /*
        对于引用类型，究竟是应该使用指针调用函数点的形式调用？这下我也懵了
        老师使用的是点,确实事实上看过大量的代码之后，也是使用点来进行调用的
        */

        epoll_addReadEvent(net_fd);   //对于读事件的监听函数
        //TCPConnection conn(net_fd);
        TCPConnectionPtr conn(new TCPConnection(net_fd));
        /*
        切记这里使用的一定是指针，因为使用TCPConnection避免不了进行拷贝操作;
        那么如果想要使用指针的话，就一定要定义堆对象，指针才有意义
        (并且这里是智能指针，智能指针基本上都是必须指向堆对象的)
        */

        /*
        另一个问题：这个conn为什么作为堆对象，要在函数内部进行定义，而不是直接定义在数据成员的位置
        这里有很多概念需要重新辨析：
        1.这里的珍宝全都没有了，自己再写一遍可能就把珍宝全都想起来了
        2.这里是因为使用堆对象，所以才有的使用
        */

    }


}
