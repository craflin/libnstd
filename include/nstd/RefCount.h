
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
        delete refObj;
    }

    Ptr& operator=(const Ptr& other)
    {
      Object* refObj;
      if(other.obj)
        refObj = other.obj, Atomic::increment(refObj->ref);
      if((refObj = obj) && Atomic::decrement(refObj->ref) == 0)
        delete refObj;
      obj = other.obj;
      return *this;
    }

    Ptr& operator=(C* obj)
    {
      Object* refObj;
      if(obj)
        refObj = obj, Atomic::increment(refObj->ref);
      if((refObj = this->obj) && Atomic::decrement(refObj->ref) == 0)
        delete refObj;
      this->obj = obj;
     return *this;
    }

    template <class D> Ptr& operator=(const Ptr<D>& other)
    {
      Object* refObj;
      if(other.obj)
        refObj = other.obj, Atomic::increment(refObj->ref);
      if((refObj = obj) && Atomic::decrement(refObj->ref) == 0)
        delete refObj;
      obj = other.obj;
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

  protected:
    virtual ~Object() {}

  private:
    usize ref;

    Object(const Object&);
    Object& operator=(const Object&);

    template<class C> friend class Ptr;
  };
};
