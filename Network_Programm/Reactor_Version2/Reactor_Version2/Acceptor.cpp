#include "Acceptor.hpp"


namespace apion {

void Acceptor::setReuseAddr(bool on){
    int judge=on;    
    /*
    定义一个int类型，来承接这个传入的bool值
    因为这个reuse函数是要求传入的必须是int类型的，还必须要进行取地址，
    所以这里才不得不既定义一个局部变量，又设计为int类型的的
    */

    int ret=setsockopt(m_sock.get_fd(),SOL_SOCKET,SO_REUSEADDR,&judge,sizeof(judge));
    /*
    所以这个函数其实也是一个典型的没说必须要调用函数来获取fd的，
    但是这里调用socket对象的函数，就完全不属于依赖关系(但是这里属于直接使用组合关系了)
    所以看来这种关系和那个非依赖关系还不一样：
    不一样体现在：这里是在类内部在定义的时候，就要求必须要调用这个函数了
    那个非依赖关系是在test程序中，才创建了两个对象，所以在test文件中才可以如此使用的，一定不是设定为了依赖关系，
    设定为了依赖关系一定是在定义类的成员函数的时候就进行定义一个局部变量的对象了

    所以两者属于完全不同的内容，原因就是一个是在test函数中才进行使用的，另一个是必须要在类定义的时候就定义需要一个别的类的对象
    */


}

void Acceptor::setReusePort(bool on){

}

void Acceptor::bind(){
    int ret=::bind(m_sock.get_fd(),(const struct sockaddr*)m_addr.get_InetAddr_ptr(),sizeof(m_addr));
    //这里的sockaddr*的*一定不能漏掉，后面获取的确实是至臻，不过这里进行强转的时候，强转为的是指针类型。所以这个不应该被漏掉
    
    if(ret<0){
        perror("bind");
    }
}

void Acceptor::listen(){
    //只要是这种同名函数，就能体现出命名空间的强大之处以及匿名命名空间的强大之处了
    //所以以后不要害怕 重名的函数，只要他俩不在一个命名空间，函数重载的概念都完全轮不上他俩
    
    int ret=::listen(m_sock.get_fd(),20000);   //我就和20000杠上了，以后只要想到监听，就立马想到20000
    if(ret<0){
        perror("listen");
    }
}

void Acceptor::accept(){

}

int Acceptor::get_fd() const{
    return m_sock.get_fd();
    //所以这里是获取acceptor本身的fd，而不是对于accept所产生的listen_fd
}

} // namespace apion