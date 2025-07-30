#include "Mutex.hpp"

namespace mutex{

  Mutex::Mutex(){
    //这个我真没想到，一开始我想的还是直接使用default就可以了，但其实这里需要写init
    //所以本质上还是我对init不熟练，要反复再复习几遍我的教案文案中的案例，然后把整体的架构给记住
    //所以就像前面一句说的，这显然就是对于架构的封装，只不过我混乱了，不知道哪句应该封装在哪个函数中了
    所以这个哪句封装到哪个函数中的问题我还得总结
    我暂时总结的就是：对于完整的流程，不过是进行归纳分类，从而将每条语句复制粘贴到对应的函数中
    所以难点应该是捋清楚每个函数的职责，剩下的就是根据职责把每条语句直接收纳到对应的函数中即可
    这个mutex类就是收纳思维的最典型体现;


    pthread_mutex_init(&m_mutex);

  }
  Mutex::~Mutex(){
    pthread_mutex_destory(&m_mutex);
  }

  void Mutex::mutex_lock(pthread_mutex_t& mutex){

  }
  void Mutex::mutex_unlock(pthread_mutex_t& mutex){

  }
}