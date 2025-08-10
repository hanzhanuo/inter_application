#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

namespace apion{

class Noncopyable{
  private:
    Noncopyable(const Noncopyable& )=delete;
    Noncopyable& operator=(const Noncopyable&)=delete;
    

  public:
    Noncopyable()=default;
    ~Noncopyable()=default;

};
}

#endif //NONCOPYABLE_H
