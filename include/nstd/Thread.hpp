
#pragma once

#include <nstd/Base.hpp>
#include <nstd/Call.hpp>

class Thread
{
public:
  Thread();
  ~Thread();

  bool start(uint (*proc)(void*), void* param);
  template <class X> bool start(X& obj, uint (X::*ptr)())
  {
    typename Call<uint>::Member<X>::Func0 func(obj, ptr);
    this->func = *(Call<uint>::Member<Thread>::Func0*)&func;
    return start((uint (*)(void*))&proc< typename Call<uint>::Member<X>::Func0 >, &this->func);
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
  template <class T> static uint proc(T* t);

  void* thread;
  Call<uint>::Member<Thread>::Func0 func;

  Thread(const Thread&);
  Thread& operator=(const Thread&);
};

template <class T> uint Thread::proc(T* t)
{
  return t->call();
}
