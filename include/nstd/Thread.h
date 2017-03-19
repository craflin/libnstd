
#pragma once

#include <nstd/Base.h>

class Thread
{
private:
  template <class X> struct MemberFuncPtr0
  {
    X* obj;
    uint (X::*ptr)();
    MemberFuncPtr0() {}
    MemberFuncPtr0(X* obj, uint (X::*ptr)()) : obj(obj), ptr(ptr) {}
    static uint call(void* p) {return (((MemberFuncPtr0*)p)->obj->*((MemberFuncPtr0*)p)->ptr)();}
  };

public:
  Thread();
  ~Thread();

  bool start(uint (*proc)(void*), void* param);
  template <class X> bool start(X* obj, uint (X::*ptr)())
  {
    MemberFuncPtr0<X> func(obj, ptr);
    this->func = *(MemberFuncPtr0<Thread>*)&func;
    return start(&MemberFuncPtr0<X>::call, &this->func);
  }

  uint join();

  static void yield();
  static void sleep(int64 milliseconds);

  /**
  * Get the id of the calling thread.
  *
  * @return The thread id.
  */
  static uint32 getCurrentThreadId();

private:
  void* thread;
  MemberFuncPtr0<Thread> func;

  Thread(const Thread&);
  Thread& operator=(const Thread&);
};
