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

    /*
    这个函数一定要助记为：紧邻着handle_NewConn()来写，因为这个函数就是专门为了handle_NewConn中的conn对象的setAllCallBacks函数服务的
    所以到时候测试用例中直接调用的是EventLoop的这个函数，而不是TCPConnection的，因为这个函数专门就是为了给conn传递，从而在EventLoop中写的函数
    所以这个函数是迫不得已产生的，是EventLoop类作为上台演讲者的使命，所以不得不设置这么一个函数
    这样助记就能明白很多了，也能顺便记住handle_NewConn该怎么写了
    */
    void EventLoop::setAllCallBacks(TCPConnectionCallBack&& cb1,TCPConnectionCallBack&& cb2,TCPConnectionCallBack&& cb3){
        m_doConnection=std::move(cb1);
        m_doInfo=std::move(cb2);
        m_doClose=std::move(cb3);
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

        epoll_addReadEvent(net_fd);   //对于内核中的红黑树结构中添加该监听
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
        2.这里是因为使用堆对象，所以才有的使用智能指针
        这里的指针是为了保证多个对象，甚至是多个类的多个对象，全都能共享使用这个TCP链接，所以才这样写的
        所以一定要熟练掌握这种思想
        */


        /*
        注意这里是调用的TCPConnection类中的函数，然后传入的是三个回调函数的萝卜坑
        所以到时候在测试用例中，肯定要手动对这三个萝卜坑进行填坑
        */
        conn->setAllCallBacks(m_doConnection,m_doInfo,m_doClose);

        /*
        注意上下两者的区别：
        上面的conn是指针所指向的一个TCP链接，
        下面的m_connMap指的是管理所有tcp链接的map
        所以想当然的，是上面那句是先实现对一个链接的"初始化"操作，下面是把初始化后的链接插入链接map中
        所以这两句是前后呼应的，和epoll的数组先手动初始化，然后再进行插入，是完全一模一样的代码逻辑
        所以现在对上面的调用setAllCallBacks函数有更深的理解了——————即这里是在进行对于每个TCP链接的初始化操作
        所以把萝卜坑填坑视为在进行初始化操作，就会对回调函数的写法的助记有更感性的理解了 
        */
        m_connMap.insert(std::make_pair(net_fd,conn));
        /*
        这里键值对一定是通过net_fd来管理和查找每个TCP链接,这个已经是我写了这么多遍网络编程所养成的通识性思维了
        所以这种map的键值对一定是fd的思想一定一定要极其熟练的肌肉反射
        一定要熟练啊，写了两遍还不熟练实在是太蠢了
        */

        conn->handleNewConnectionCallback();
        /*
        上面两个是初始化并入map，这一步是入map之后屁股还没坐热乎，就立刻进行对初始化的内容的使用了
        所以这里调用的函数就是初始化的时候用于填坑的函数
        所以这个函数是变用来对新链接初始化，边直接对新链接使用的
        上面这一行的思想理解明白之后，就能助记住这个函数的代码逻辑该怎么写了
        */

    }


    void EventLoop::handle_OldConn(int fd){
        auto it=m_connMap.find(fd);
        if(it!=m_connMap.end()){  //表示此时查找成功
            bool isclose=it->second->isClosed();
            if(isclose){   //表示此时如果已经断开连接了
                /*
                已经断开链接的代码逻辑一定要和：上面新链接的初始化方法的代码逻辑进行直接的对比
                对比就会发现两者之间有非常类似的对应操作关系和顺序
                都是：
                1.对于回调函数萝卜坑调用
                2.对于红黑树进行处理
                3.对于map进行处理
                唯一的不同就是对于map进行处理的时候，新链接的操作是需要先初始化setAllcallback——————这就是这两者唯一的不同了
                */
               it->second->handleCloseCallback();

               //ctl函数中的删除操作，即从内核中的红黑树结构中删除该监听
               epoll_delReadEvent(fd);

               //从TCP链接们的map结构中删除
               m_connMap.erase(fd);


            }else{   //if-else结构一般都是把压轴的(or可以说是正确主流的情况)放在最后面
               
                /*
                处理正常的结构反而是代码逻辑最简单的
                (其实仅仅是看起来最简单，但其实handleMessageCallback的代码逻辑绝对是最难写的)
                所以它仅仅是调用简单，但是其函数内部实现其实是最难的
                */
                it->second->handleMessageCallback();
            }
        }
    }


}
