
#pragma once

#include <nstd/Atomic.h>

class RefCount
{
public:
  template <class T> class Ptr
  {
  public:
    Ptr() : obj(0) {}
    Ptr(T* obj) : obj(obj)
    {
      if(obj)
      {
        Object* refObj = obj;
        Atomic::increment(refObj->ref);
      }
    }
    Ptr(const Ptr& other) : obj(other.obj)
    {
      if(obj)
      {
        Object* refObj = obj;
        Atomic::increment(refObj->ref);
      }
    }

    ~Ptr()
    {
      Object* refObj = obj;
      if(refObj && Atomic::decrement(refObj->ref) == 0)
        delete obj;
    }

    Ptr& operator=(T* obj)
    {
      Object* refObj = this->obj;
      if(refObj && Atomic::decrement(refObj->ref) == 0)
        delete this->obj;
     if((this->obj = obj))
     {
        Object* refObj = obj;
        Atomic::increment(refObj->ref);
     }
    }

    Ptr& operator=(const Ptr& other)
    {
      Object* refObj = obj;
      if(refObj && Atomic::decrement(refObj->ref) == 0)
        delete obj;
     if((obj = other.obj))
     {
        Object* refObj = obj;
        Atomic::increment(refObj->ref);
     }
     return *this;
    }

    bool operator==(const Ptr& other) const {return obj == other.obj;}
    bool operator!=(const Ptr& other) const {return obj != other.obj;}

  private:
    T* obj;
  };

  class Object
  {
  public:
    Object() : ref(0) {}

  private:
    usize ref;

    template<class T> friend class Ptr;
  };
};
