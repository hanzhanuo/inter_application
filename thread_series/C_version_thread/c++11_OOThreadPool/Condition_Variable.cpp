#include "Condition_Variable.hpp"

namespace ghz{

   Condition::Condition(pthread_mutex_t& mutex)
      :m_mutex(mutex){
      pthread_cond_init(&m_cond,nullptr);
      //条件变量，互斥锁，线程这三个的初始化都有个参数是特征参数，这个参数都是没用，都是无脑填nullptr即可
   }
   Condition::~Condition(){
      pthread_cond_destroy(&m_cond);
   }
   void Condition::wait(){
        pthread_cond_wait(&m_cond,&m_mutex);
        //wait填互斥锁是为了体现进行上下两部分的时候都是需要对对应锁进行上锁和解锁的
        //所以才必须要指定锁参数
   }
   void Condition::notify_one(){
      pthread_mutex_signal(&m_cond);
      //切记下面这两个都是只能填条件变量，不能填互斥锁，并且填的也不是具体释放哪个线程
      //这个条件变量的针对主体是谁之前已经分析过了，所以这里就不再说一遍了
      
   }
   void Condition::notify_all(){
      pthread_cond_broadcast(&m_cond);
      //这里经常拼写错误，记住是不饶的
   }
}