//#include <stdlib.h>
//这个类似乎不需要写任何的头文件，因为它什么都不需要使用——————其实很多的对于类进行声明的头文件，由于不需要进行实现，所以都是不需要包含很多头文件的
//甚至对于这些写类的头文件，都是可以避开对其他头文件进行包含，而是直接通过前向声明的方式避免循环声明问题





//对于这些所有的类的头文件的编写，都使用：先命名空间，再类的写法进行记忆即可
namespace ghz{

class Noncopyable{

  private:
    Noncopyable(const Noncopyable&)=delete;
    Noncopyable& operator=(const Noncopyable&)=delete;

   //由于这里进行的是删除操作，无需调用。只要不需要被外部调用，就最好放在private中
   //所以以后记住只要是delete的内容都放在private中即可

  public:
   //记住了两个delete放在private中了，剩下两个就能排除法记住放在哪了
    Noncopyable()=default;
    ~Noncopyable()=default;

};

}