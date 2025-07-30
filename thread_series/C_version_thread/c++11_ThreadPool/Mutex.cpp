#include "Mutex.hpp"

namespace ghz{

  Mutex::Mutex(){

    pthread_mutex_init(&m_mutex,nullptr);
    //初始化的时候又是传互斥锁的特征，所以就create一样，只要涉及到传特征的，就暂定写nullptr即可

  }
  Mutex::~Mutex(){
    pthread_mutex_destory(&m_mutex);
  }

  void Mutex::mutex_lock(){
    pthread_mutex_lock(&m_mutex);
  }
  void Mutex::mutex_unlock(){
    pthread_mutex_unlock(&m_mutex);
  }
}