//每个都少了避免重复引用的宏定义了

#include <pthread.h>

namespace ghz{

class Condition{

   private:
    pthread_cond_t m_cond;

   public:
   Condition();
   //~Condition()=default;
   ~Condition();
   //这里的构造函数和析构函数是用来加锁和解锁的，这个一定不能忘了 

   void wait(pthread_cond_t&,pthread_mutex_t&);
   void notify_one(pthread_cond_t&);
   void notify_all(pthread_cond_t&);

   

   
   
}
}