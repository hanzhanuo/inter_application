#include "Condition_Variable.hpp"

namespace ghz{

   Condition::Condition(){
      pthread_cond_init()
   }
   Condition::~Condition(){

   }
   void Condition::wait(pthread_cond_t& cond,pthread_mutex_t& mutex){
        pthread_cond_wait(cond,mutex);
   }
   void Condition::notify_one(pthread_cond_t&){

   }
   void Condition::notify_all(pthread_cond_t&){

   }
}