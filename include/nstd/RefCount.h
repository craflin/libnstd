
#pragma once

#include <nstd/Atomic.h>

class RefCount
{
public:
  class Object;

  template <class C = Object> class Ptr
  {
  public:
    Ptr() : obj(0) {}

    Ptr(const Ptr& other) : obj(other.obj)
    {
      if(obj)
      {
        Object* refObj = obj;
        Atomic::increment(refObj->ref);
      }
    }

    template <class D> Ptr(D* obj) : obj(obj)
    {
      if(obj)
      {
        Object* refObj = obj;
        Atomic::increment(refObj->ref);
      }
    }

    template <class D> Ptr(const Ptr<D>& other) : obj(other.obj)
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

    Ptr& operator=(C* obj)
    {
      Object* refObj = this->obj;
      if(refObj && Atomic::decrement(refObj->ref) == 0)
        delete this->obj;
     if((this->obj = obj))
     {
        refObj = obj;
        Atomic::increment(refObj->ref);
     }
     return *this;
    }

    template <class D> Ptr& operator=(const Ptr<D>& other)
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

    C& operator*() const {return *obj;}
    C* operator->() const {return obj;}

    template <class D> bool operator==(D* other) const {return obj == other;}
    template <class D> bool operator!=(D* other) const {return obj != other;}
    template <class D> bool operator==(const Ptr<D>& other) const {return obj == other.obj;}
    template <class D> bool operator!=(const Ptr<D>& other) const {return obj != other.obj;}

    operator bool() const {return obj != 0;}

    void swap(Ptr& other)
    {
      C* tmp = other.obj;
      other.obj = obj;
      obj = tmp;
    }

  private:
    C* obj;

    template<class D> friend class Ptr;
  };

  class Object
  {
  public:
    Object() : ref(0) {}
    virtual ~Object() {}

  private:
    usize ref;

    template<class C> friend class Ptr;
  };
};
