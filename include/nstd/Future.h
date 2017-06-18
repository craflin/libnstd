
#pragma once

#include <nstd/Signal.h>

template <typename T> class Future;

template <> class Future<void>
{
public:
  Future() : sig(true), aborting(false), state(idleState) {}
  ~Future() {join();}

  template <typename P, typename A> void start(void (*func)(P), const A& a)
  {
    struct FuncArgs
    {
      Future& x;
      void (*y)(P);
      A a;
      FuncArgs(Future& x, void (*y)(P), const A& a) : x(x), y(y), a(a) {}
      static void proc(FuncArgs* p)
      {
        p->y(p->a);
        p->x.set();
        delete p;
      }
    };
    startProc((void (*)(void*))&FuncArgs::proc, new FuncArgs(*this, func, a));
  }

  void abort() {aborting = true;}
  bool isAborting() const {return aborting;}
  bool isFinished() const {return state == finishedState;}
  bool isAborted() {return state == abortedState;}
  void join() {sig.wait();}

protected:
  enum State
  {
    idleState,
    runningState,
    finishedState,
    abortedState,
  };

protected:
  Signal sig;
  volatile bool aborting;
  volatile int state;

protected:
  void set();
  void startProc(void (*proc)(void*), void* param);

private:
  Future(const Future&);
  Future& operator=(const Future&);

  class Private;
};

template <typename T> class Future : private Future<void>
{
public:
  Future() {}
  ~Future() {join();}

  template <typename A> void start(T (*func)(const A&), const A& a)
  {
    struct FuncArgs
    {
      Future& x;
      T (*y)(const A&);
      A a;
      FuncArgs(Future& x, T (*y)(const A&), const A& a) : x(x), y(y), a(a) {}
      static void proc(FuncArgs* p)
      {
        p->x.set(p->y(p->a));
        delete p;
      }
    };
    startProc((void (*)(void*))&FuncArgs::proc, new FuncArgs(*this, func, a));
  }

  void abort() {aborting = true;}
  bool isAborting() const {return aborting;}
  bool isFinished() const {return state == finishedState;}
  bool isAborted() {return state == abortedState;}
  void join() {sig.wait();}

  operator const T&() const {return result;}

private:
  T result;

private:
  void set(const T& val);

  Future(const Future&);
  Future& operator=(const Future&);
};

template <typename T> void Future<T>::set(const T& val)
{
    result = val;
    Future<void>::set();
}
