#include <pthread.h>

namespace mutex{   
//我知道了，应该是为了避免和std中的重名导致的问题，所以只要是写自己造轮子，就最好写为命名空间的写法


class Mutex{
 private:

  //因为是造轮子的封装法，所以数据成员一定是把_t类型的最原始数据类型放进来
  pthread_mutex_t m_mutex;

 public:
  Mutex();
  ~Mutex();

  void mutex_lock(pthread_mutex_t& mutex);
  void mutex_unlock(pthread_mutex_t& mutex);

};
}