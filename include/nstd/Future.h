
#pragma once

#include <nstd/Thread.h>
#include <nstd/Signal.h>

template <typename T> class Future
{
public:
  Future() : sig(true), aborting(false), state(idleState) {}
  ~Future() {join();}

  template <typename A> void start(T (*func)(const A&), const A& a)
  {
    struct FuncArgs
    {
      Future& x;
      T (*y)(const A&);
      A a;
      FuncArgs(Future& x, T (*y)(const A&), const A& a) : x(x), y(y), a(a) {}
      static uint proc(void* p)
      {
        FuncArgs* z = (FuncArgs*)p;
        z->x.set(z->y(z->a));
        delete z;
        return 0;
      }
    };
    join();
    aborting = false;
    state = runningState;
    sig.reset();
    FuncArgs* z = new FuncArgs(*this, func, a);
    if(!thread.start(&FuncArgs::proc, z))
      FuncArgs::proc(z);
  }

  void abort() {aborting = true;}
  bool isAborting() const {return aborting;}
  bool isFinished() const {return state == finishedState;}
  bool isAborted() {return state == abortedState;}
  void join() {sig.wait();}

  operator const T&() const {return result;}

private:
  enum State
  {
    idleState,
    runningState,
    finishedState,
    abortedState,
  };

private:
  T result;
  Thread thread;
  Signal sig;
  volatile bool aborting;
  volatile int state;

private:
  void set(const T& val)
  {
    if(aborting)
      Atomic::swap(state, abortedState);
    else
    {
      result = val;
      Atomic::swap(state, finishedState);
    }
    sig. set();
  }

  Future(const Future&);
  Future& operator=(const Future&);
};
