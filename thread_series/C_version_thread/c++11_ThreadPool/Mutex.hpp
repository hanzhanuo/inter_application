#include <pthread.h>

namespace ghz{   
//我知道了，应该是为了避免和std中的重名导致的问题，所以只要是写自己造轮子，就最好写为命名空间的写法


class Mutex
:public Noncopyable{
 private:

  //因为是造轮子的封装法，所以数据成员一定是把_t类型的最原始数据类型放进来
  pthread_mutex_t m_mutex;

 public:
  Mutex();
  ~Mutex();

  // void mutex_lock(pthread_mutex_t& mutex);
  // void mutex_unlock(pthread_mutex_t& mutex);

  void mutex_lock();
  void mutex_unlock();
  //这里不应该写参数的。我这里写参数还是因为沉醉于面向过程的没有数据成员这样的全局变量的思想中
  //但是这里由于已经有全局数据成员可以使用了，并且又是简单的封装而不是代码逻辑处理的问题，所以更不需要传参数了
  //所以以后记住：当进行复杂代码逻辑的时候，才可能涉及到传参。这里这种只进行一步封装的函数设计，压根不需要想传参的问题

};
}