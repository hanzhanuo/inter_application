#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

namespace apion{


/*
   构造函数，析构函数，拷贝构造函数，赋值运算符函数这些都不能被继承

 * */
class Noncopyable{
  private:
    Noncopyable(const Noncopyable& )=delete;
    Noncopyable& operator=(const Noncopyable&)=delete;
    //四大函数的助记：首先起手的都一定是类数据类型作为返回值，这个是我忘光了的起始

  public:
    Noncopyable()=default;
    ~Noncopyable()=default;

};
}

#endif //NONCOPYABLE_H
