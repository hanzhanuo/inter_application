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
                int fd=m_evtArr[idx].data.fd;

                /*
                这里是又一层if-else判断，用于判断是一些特殊的链接，还是只是一些普通的读写链接
                所以这里就必须要记住：这个wait函数的处理结构是：epoll循环内判断，判断内再接循环，循环内再接判断
                一定要把这种结构给助记清楚，不然这个函数实在是不好写
                */
                if(fd==m_acceptor.get_fd()){   //如果是accept的fd有读就绪
                    
                    handle_NewConn();   //这个就是下面的处理新链接的成员函数操作

                }else{
                    handle_OldConn(fd);
                }

            }
        }
        
    }

    
    void EventLoop::setAllCallBacks(TCPConnectionCallBack&& cb1,TCPConnectionCallBack&& cb2,TCPConnectionCallBack&& cb3){
        m_doConnection=std::move(cb1);
        m_doInfo=std::move(cb2);
        m_doClose=std::move(cb3);
    }


    void EventLoop::handle_NewConn(){
        
        

        int net_fd=m_acceptor.accept();   
        

        epoll_addReadEvent(net_fd);   //对于内核中的红黑树结构中添加该监听
        //TCPConnection conn(net_fd);
        TCPConnectionPtr conn(new TCPConnection(net_fd));
        /*
        切记这里使用的一定是指针，因为使用TCPConnection避免不了进行拷贝操作;
        那么如果想要使用指针的话，就一定要定义堆对象，指针才有意义
        (并且这里是智能指针，智能指针基本上都是必须指向堆对象的)
        */

        


        /*
        注意这里是调用的TCPConnection类中的函数，然后传入的是三个回调函数的萝卜坑
        所以到时候在测试用例中，肯定要手动对这三个萝卜坑进行填坑
        */
        conn->setAllCallBacks(m_doConnection,m_doInfo,m_doClose);

        
        m_connMap.insert(std::make_pair(net_fd,conn));
        

        conn->handleNewConnectionCallback();
        

    }


    void EventLoop::handle_OldConn(int fd){
        auto it=m_connMap.find(fd);
        if(it!=m_connMap.end()){  //表示此时查找成功
            bool isclose=it->second->isClosed();
            if(isclose){   //表示此时如果已经断开连接了
                
               it->second->handleCloseCallback();

               //ctl函数中的删除操作，即从内核中的红黑树结构中删除该监听
               epoll_delReadEvent(fd);

               //从TCP链接们的map结构中删除
               m_connMap.erase(fd);


            }else{   //if-else结构一般都是把压轴的(or可以说是正确主流的情况)放在最后面
               
               
                it->second->handleMessageCallback();
            }
        }
    }


}
