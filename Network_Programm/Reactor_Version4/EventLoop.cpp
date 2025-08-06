#include "EventLoop.hpp"
#include <sys/eventfd.h>

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


    //只有在写头文件的时候，才需要进行："每个模块需要写在一起"这样的设计，在实现文件中随便写即可
    int EventLoop::createEventfd(){
        int fd=eventfd(0,0);   //第一个参数是初始化计数器的值，第二个参数作为标志位必须写为0
        
        /*
        这里写为fd，而不是event_fd,还是因为函数封装的思想
        因为外部只需要用一个event_fd来承接就可以了，函数内部写什么名是不需要体面的
        所以这种对于封装的内部无需体面的思想我见识过很多次了，但是始终没有变成我的思想
        */
        
        if(fd<0){
            perror("eventfd");
            return -1;
        }

        return fd;
    }   

    void EventLoop::handleReadEvent(){   //进行read操作
        uint64_t howmany=0;   
        //这个howmany是必须要有的，是一个buf，用于承接计数器中此次读出来的计数器的值是多少
        //在这里这个buf的值是用来查看是否是每次都传一次读一次的，避免造成传多次读一次，从而造成通信的信息丢失
        //通信的收发次数问题，是本地的线程间通信中，最最重要的课题，所以每次都需要先条件反射的思考有没有可能这种情况，从而提升代码的健壮性
        int ret=read(m_eventfd,&howmany,sizeof(howmany));
        printf("the read count is %1d\n",howmany);
        if(ret!=sizeof(howmany)){
            perror("read");
        }
    }   
    void EventLoop::do_wakeup(){  //write操作，通过写操作使得内核计数器+1
        uint64_t write_count=1;
        int ret=write(m_eventfd,&write_count,sizeof(write_count));
        if(ret!=sizeof(write_count)){
            perror("write");
        }
    }   
    void EventLoop::do_PendingFunctors(){  //对vector的内容进行处理的操作
        printf("doPendingFuntor");

        /*
        这里的处理思路是使用一个tmp进行处理操作，然后让原本的vector继续进行接收内容的操作
        所以其实这里从解决方法的思想上来看，是和select的解决方法是一模一样的
        那么这种解决方法一般都是面对什么问题提出的呢？
        答：是根据一些读写冲突——————主要是只需要写实时写操作修改某个大型内容的时候(尤其是对于容器的时候)
        */

        std::vector<Functor> tmp;
        /*
        tmp=   
        注意：对于vector的拷贝操作使用的是等号不假，但是这里进行的是交换操作
        ——————即把pending的vector搬空，然后让这个被搬空的再去读
        所以这种搬空法和select的同步法还是有细微差距的，这个差距和规律无关，和具体的业务逻辑场景有关
        不同的业务逻辑场景对于vector的处理方法会有策略上的不同
        */
    
    
    {   //为了尽可能缩减智能锁的粒度，以及对别的内容的锁住的影响，所以这里写为语句块的形式
        //语句块能实现——————出了这个语句块，锁的生命周期就立马结束了

    MutexLockGuard autolock(m_mutex);
        /*
        所以这里又是——————既涉及到了之前总结的完全解耦，所以必须要传入具体是对哪个mutex进行的智能锁升级
        又涉及到了这种智能锁的生命周期最好最好是局部函数这种作用域的，最好不要是全局作用域的，否则就违反了RAII了

        */
       tmp.swap(m_PendingFunctor);
    
    }   //end of 语句块    所以以后每个智能锁都要直接放在语句块中


    for(auto& functor:tmp){
        functor();
    }
        
    }   
    void EventLoop::run_inLoop(Functor&& cb){  //对于vector进行插入的操作

    }   


}
