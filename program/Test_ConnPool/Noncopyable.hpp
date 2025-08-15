#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

namespace group_6{

  enum{
    READ=1,
    WRITE
};

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
