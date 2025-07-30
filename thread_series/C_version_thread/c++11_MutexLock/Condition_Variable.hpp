//每个都少了避免重复引用的宏定义了

#include <pthread.h>

namespace condition{

class Condition{

   private:
    pthread_cond_t m_cond;

   public:
   Condition();
   ~Condition();

   void wait(pthread_cond_t&,pthread_mutex_t&);
   void notify_one(pthread_cond_t&);
   void notify_all(pthread_cond_t&);
}
}