
#pragma once

#include <nstd/Signal.h>

template <typename T> class Future;

template <> class Future<void>
{
public:
  Future() : sig(true), aborting(false), state(idleState) {}
  ~Future() {join();}

protected:
  template <typename X> struct Func;
  /*
  {
    template <typename P, typename A> struct Args1
    {
      Future<X>& x;
      X (*y)(P);
      A a;
      Args1(Future<X>& x, X (*y)(P), const A& a) : x(x), y(y), a(a) {}
      X call() {return y(a);}
      static void proc(Args1* p) {p->x.callAndSet(p); delete p; }
    };
  };
  */
public:
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

  template <typename P, typename A> void start2(void (*func)(P), const A& a)
  {
    startProc((void (*)(void*))&Func<void>::Args1<P, A>::proc, new Func<void>::Args1<P, A>(*this, func, a));
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

  template <typename A> void callAndSet(A* a)
  {
    a->call();
    set();
  }

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

  template <typename P, typename A> void start(T (*func)(P), const A& a)
  {
    struct FuncArgs
    {
      Future& x;
      T (*y)(P);
      A a;
      FuncArgs(Future& x, T (*y)(P), const A& a) : x(x), y(y), a(a) {}
      static void proc(FuncArgs* p) {p->x.set(p->y(p->a)); delete p;}
    };
    startProc((void (*)(void*))&FuncArgs::proc, new FuncArgs(*this, func, a));
  }


  template <typename P, typename A> void start2(T (*func)(P), const A& a)
  {
    startProc((void (*)(void*))&Func<T>::Args1<P, A>::proc, new Func<T>::Args1<P, A>(*this, func, a));
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

  template <typename A> void callAndSet(A* a)
  {
    set(a->call());
  }

  Future(const Future&);
  Future& operator=(const Future&);

  friend struct Func<T>;
};

template <typename T> void Future<T>::set(const T& val)
{
    result = val;
    Future<void>::set();
}


  template <typename X> struct Future<void>::Func
  {
    template <typename P, typename A> struct Args1
    {
      Future<X>& x;
      X (*y)(P);
      A a;
      Args1(Future<X>& x, X (*y)(P), const A& a) : x(x), y(y), a(a) {}
      X call() {return y(a);}
      static void proc(Args1* p) {p->x.callAndSet(p); delete p; }
    };
  };
