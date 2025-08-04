#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "func.h"
#include "Socket.hpp"
#include "InetAddress.hpp"
#include <string>


using namespace std;


namespace apion{

class Acceptor {    //单纯用来实现链接的，不需要管收发的操作。所以是对于send，recv之上的内容的拆分出来封装
  
  /*
  就是这个类中，有几个非常重要的点：
  1.为什么要把accept函数从ready函数中分离出来？
  原因都体现在了test函数中——————即test函数中的accept函数是被connector调用的
  所以这体现了先connect再accept的行为顺序。所以自然不能等客户端都connect了，这个服务器端还没调用ready函数
  所以才导致了accept函数必须要和ready函数进行分离
  
  
  2.为什么不是依赖关系
  我明白了!   因为依赖关系都是在参数列表中or返回值中传入对象(即局部变量生命周期的对象)
  但是这里仅仅是进行对象的成员函数的调用。所以两者之间不是依赖关系
  所以对于类和类之间的关系的理解更加深了一步————————即只要不是必须要直接使用类，如果是需要使用类的成员函数，那么就不算类和类之间的关系
  最最典型的甚至都不是必须要使用getter函数(因为需要使用getter函数意味着需要先拥有这个类)
  而是像这里的例子这样——————本来参数列表中定义的就是一个int，而不是别的类的成员，所以这里只是刚好使用了这个成员函数，不使用这个成员函数，这个类的传参也不会没法传了
  所以可以理解为甚至都没说非要你帮忙。没有迫切求着别人帮忙的，就根本不算依赖关系
  这里就是没有求着你的accept函数传，我自己手写传也可以。所以这里就压根算不上依赖关系
  */

  private:

  void setReuseAddr(bool);
  void setReusePort(bool);
  //我又一次把这个bool给忘了，这个bool千万不能忘
  void bind();
  void listen();

  Socket m_sock;    
  /*
  这里由于socket不能被copy，所以拥有socket的acceptor类的对象就也不能被copy
  这个是什么原理？为什么这个就也不能被复制了？
  好像是因为赋值一个对象的时候，就要复制这个对象的所有状态，所以状态不可复制对象就业不可复制
  所以这里引出来了另一个不可复制的问题——————即不仅是继承之后不可复制，直接组合关系拥有该对象的时候，也是不可复制的
  所以这种不可复制属性的两种蔓延方法一定要熟练
  */
  InetAddress m_addr;
  

  public:
  
  Acceptor(unsigned short port,const string& ip="0.0.0.0");
  /*
  这种一个类中包含了另一个类的对象(组合关系)，从而另一个类的对象需要靠这个类传参进行初始化
  这个场景在线程池问题中讲过，属于非常经典的:"组合关系就会导致需要参数如此传递"的问题
  */

  void ready(){
    //所以这再一次证明了——————只要有前向声明，即使函数还没有实现，也不用担心，直接调用即可
    setReuseAddr(true);
    setReusePort(true);
    bind();
    listen();
  }
  /*
  因为原本这个类设计的目的就是尽可能的把建立链接的步骤一步到位，所以这里封装一个一步到位的函数，就不用手动调用多个函数了
  所以这种"类为了封装操作，所以在类内就设计一个函数来封装操作"的写法，也可以说是木偶行为的第二大种类——————对自己的类一下封装许多操作的捆绑木偶行为
  */


  void accept();
  //"没和ready封装起来，而是剥离出来了"的原因已经在上面说完了


};

}
