#ifndef __Noncopyable_H__
#define __Noncopyable_H__


//首先要再明确一遍这个头文件的作用是什么：
//这个头文件是为了创建一个可被继承的防止对系统资源进行拷贝的类
//所以它这个头文件的内容就必定是个类，这样想就能立刻想到该如何设计内部的实现内容了
//所以能想明白内部实现是什么样的，就是对于类内部设计的第二个关键步骤

namespace noncopyable {
class Noncopyable {

    private:
    Noncopyable(const Noncopyable&)=delete;
    Noncopyable& operator=(const Noncopyable&)=delete;

    public:
    Noncopyable()=default;
    ~Noncopyable()=default;

    //以后写这种类就无脑这样六行写即可
    

}
}

#endif // __Noncopyable_H__