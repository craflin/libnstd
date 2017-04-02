
#pragma once

#include <nstd/Atomic.h>

class RefCount
{
public:
  class Object;

  template <class C = Object> class Ptr
  {
  public:
    Ptr() : refObj(0), obj(0) {}

    Ptr(const Ptr& other) : refObj(other.refObj), obj(other.obj)
    {
      if(refObj)
        Atomic::increment(refObj->ref);
    }

    template <class D> Ptr(D* obj) : refObj(obj), obj(obj)
    {
      if(refObj)
        Atomic::increment(refObj->ref);
    }

    template <class D> Ptr(const Ptr<D>& other) : refObj(other.refObj), obj(other.obj)
    {
      if(refObj)
        Atomic::increment(refObj->ref);
    }

    ~Ptr()
    {
      if(refObj && Atomic::decrement(refObj->ref) == 0)
        delete refObj;
    }

    Ptr& operator=(const Ptr& other)
    {
      if(other.refObj)
        Atomic::increment(other.refObj->ref);
      if(refObj && Atomic::decrement(refObj->ref) == 0)
        delete refObj;
      refObj = other.refObj;
      obj = other.obj;
      return *this;
    }

    Ptr& operator=(C* obj)
    {
      Object* refObj = obj;
      if(refObj)
        Atomic::increment(refObj->ref);
      if(this->refObj && Atomic::decrement(this->refObj->ref) == 0)
        delete this->refObj;
      this->refObj = refObj;
      this->obj = obj;
     return *this;
    }

    template <class D> Ptr& operator=(const Ptr<D>& other)
    {
      if(other.refObj)
        Atomic::increment(other.refObj->ref);
      if(refObj && Atomic::decrement(refObj->ref) == 0)
        delete refObj;
      refObj = other.refObj;
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
    Object* refObj;
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
