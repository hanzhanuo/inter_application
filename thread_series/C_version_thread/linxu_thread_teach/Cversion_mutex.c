#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>   //只要是linux中的线程问题，所有的都是使用这个库来实现的
#include <unistd.h>  

//下面这些库是为了实现各种文件操作的。所以和文件有关的时候才会使用这些库
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

//对于条件变量的核心，就是wait的上半部和下半部问题
//这个代码的教案功能主要是对于互斥锁和条件变量的应用


//互斥锁和条件变量都是最好设置成全局的
pthread_mutex_t mutex;
pthread_cond_t cond;





void* pthread_innerfunc1(void* arg){   //这个arg是一定要填的，是用来进行create的变量承接的


    //在哪个线程内部调用这个函数，就是对哪个线程进行加解锁，休眠唤醒操作。
    //这可以说是相当于这四个操作函数的共同点——————都对是哪个线程默认为是当前线程
    

    //对于所有对的子线程的通用写法(之前只归纳到了push和pop的通用写法，现在来看还可以再通用一些)
    /*
    通用写法就是：先写代码逻辑，再对系统资源代码逻辑进行加解锁，最后再写休眠和唤醒   
    */

    pthread_mutex_lock(&mutex);
    printf("func1's core code");   //用这个来表示这是线程1的核心代码逻辑
    pthread_mutex_unlock(&mutex);
    
    
    pthread_cond_wait(&cond,&mutex);    
    /*
    对于wait函数有几个比较重要的特点：
    1.wait函数一定是在哪个线程中调用，就对哪个线程休眠在这条语句
    2.对于wait函数，一定要前面上锁后面解锁(注意这里的上锁解锁是为wait函数提供的，不是为系统资源准备的)
    3.当wait被唤醒后，如果抢不到锁，还是会一直卡在这句wait代码上，直到抢到锁才能return(这种卡在这条语句上的操作，和select一模一样)
    4.第四条写在笔记上了，主要内容就是无法像在同一个线程内加解锁一样在同一个线程中休眠唤醒

    */

    pthread_exit(NULL);
}


void* pthread_innerfunc2(void* arg){

    // pthread_mutex_t mutex1;
    // pthread_cond_t cond1;
    //这里的条件变量和互斥锁都是在函数内定义的，所以是局部变量
    //所以这些变量只在函数内有效，函数外无法访问到这些变量
    //所以这些变量只能在函数内使用，不能在函数外使用
    //上面这些注释告诉了我：想要真正的使用条件变量和互斥锁影响，就必须把这些内容设置为全局的内容
    //像这里这样设置成局部变量只能自己线程内部过家家，其他线程都使用不到


    pthread_mutex_lock(&mutex);
    printf("func2's core code");   
    pthread_mutex_unlock(&mutex);

    //pthread_cond_broadcast(&cond);
    pthread_cond_signal(&cond);
    //一个极其重大的误区——————我一直认为这里的参数列表是对于哪个线程进行唤醒的，但是实际上这里的参数列表是对于哪个条件变量进行唤醒的
    //所以这里的参数列表是一个条件变量的地址，而不是一个线程的地址
    //所以这里的效果是在这个参数列表上等待的线程是单个被唤醒，还是全部被唤醒。而不是在参数列表中指定对于哪个线程进行唤醒

    pthread_exit(NULL);
    //由于子线程入口函数一定是void*,所以这里和入口函数的形参列表一样，也是void*作为万能钥匙,传入任何类型的返回值都可以
    //但是一定要对join中用来承接返回值的变量也设置对应的数据类型才可以(这点和入口函数的void*一进来就首先要进行强转一起记会更好记)

    //并且其实从这个函数就开始了——————即即使不用传是哪个线程，也能知道就是默认当前这个线程
}


int main(int argc,char** argv){
  //只有对于局部的互斥锁和条件变量，才会在每个子线程内部进行初始化和销毁，否则就都是无脑在main函数中进行初始化和销毁  
  //先无脑进行互斥锁和条件变量的初始化和销毁放置在最外层结构

  pthread_mutex_init(&mutex,NULL);   //初始化的时候无脑填NULL,销毁的时候甚至都不需要NULL，只需要互斥锁一个参数即可
  pthread_cond_init(&cond,NULL);

  pthread_t thread_id1,thread_id2;    
  //在定义的时候尽可能对这两个变量加id命名，否则容易导致忘记它仅仅是为了填到线程创建中的了
  //并且这两个内容没必要初始化，把create当作初始化就可以了

  pthread_create(thread_id1,NULL,pthread_innerfunc1,NULL);
  pthread_create(thread_id2,NULL,pthread_innerfunc2,NULL);
  //这里一定要形成助记：即对于create和join一定是同时写的，并且这两者都是有返回值可以用来进行错误检测的
  //这样一下记两个函数，整个主线程的必备框架就一下就搭好了(因为记这两个函数又能连带着记住exit函数)






  pthread_join(thread_id1,NULL);
  //然后join函数还可以再对子线程的exit的返回值进行承接。所以就可以继续顺藤摸瓜写子线程的exit的框架了
  //并且记住这里的承接返回值：既是二级指针，又是一种由于是返回值所以是值传递拷贝承接的效果
  //这里如果想要填承接值的话，一定要填void类型的二级指针。因为void*是类似万能钥匙的效果

  pthread_join(thread_id2,NULL);  
  //void*和void**本质上都是指针，所以这两种类型的形参列表传NULL一定不会出错


  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
  //每次在写同步互斥问题的时候，都先这样把互斥锁和条件变量的框架给搭好，
  //然后剩下的就是：先写代码逻辑，然后再把在什么位置加解锁&睡眠唤醒给添加进去即可
}