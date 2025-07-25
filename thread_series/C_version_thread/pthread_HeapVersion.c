#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

//这个程序是使用堆对象实现对于两套系统的参数传递，以及对于void*进行各种类型的转换操作练习

void* start_routine(void* arg) {
    int* num = (int*)arg;  //将void*转换为int*

    *num += 100;  
    //对num进行操作，所以各种强转后操作的方法就是：在进行强转之后，进行解引用然后再进行各种操作
    //所以以后对于void*类型的指针的操作，都是先进行强制类型转换，然后再进行解引用操作这样的两步走


    printf("Thread: Number is %d\n", *num);
    
    pthread_exit(NULL);  //退出线程
}


int main(int argc,char** acgv){  //这种写法表示指针指向一个字符串数组


    pthread_t pthread_id = 0;  
    pthread_create(&pthread_id, NULL, start_routine, acgv[1]);
    //这里的acgv[1]表示传入的第一个参数(即命令行参数)，这里的参数是一个字符串，所以需要进行强制类型转换为int*类型
    
    return 0;
}