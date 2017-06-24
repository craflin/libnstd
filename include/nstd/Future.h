
#pragma once

#include <nstd/Signal.h>

template <typename T> class Future;

template <typename A> struct Call // A
{
  template <class C> struct Member // C
  {
    template <typename D> struct Func1 // D E F G H I J K L M
    {
      C* c;
      A (C::*a)(D);
      A call(D d) {return c->*a(d);}
    };

    template <typename D, typename P> struct Args1 : public Func1<D> // P Q R S T U V W X Y
    {
      P p;
      void* z;
      A call() {return Func1<D>::c->*Func1<D>::a(p);}
    };
  };
 
  template <typename D> struct Func1 // D E F G H I J K L M
  {
    A (*a)(D);
    A call(D d) {return a(d);}
    Func1(A (*a)(D)) : a(a) {}
  };

  template <typename D, typename P> struct Args1 : public Func1<D> // P Q R S T U V W X Y
  {
    P p;
    void* z;
    A call() {return Func1<D>::a(p);}
    Args1(A (*a)(D), const P& p, void* z) : Func1<D>(a), p(p), z(z) {}
  };
};

template <> class Future<void>
{
public:
  Future() : sig(true), aborting(false), state(idleState) {}
  ~Future() {join();}

public:
  void abort() {aborting = true;}
  bool isAborting() const {return aborting;}
  bool isFinished() const {return state == finishedState;}
  bool isAborted() const {return state == abortedState;}
  void join() {sig.wait();}

  template <typename P, typename A> void start(void (*func)(P), const A& a)
  {
    startProc((void (*)(void*))&proc< Call<void>::template Args1<P, A> >, new Call<void>::template Args1<P, A>(func, a, this));
  }

private:
  enum State
  {
    idleState,
    runningState,
    finishedState,
    abortedState,
  };

private:
  Signal sig;
  volatile bool aborting;
  volatile int state;

  void set();

  void startProc(void (*proc)(void*), void* param);

private:
  template <class A> static void proc(A* a);

  Future(const Future&);
  Future& operator=(const Future&);

  class Private;
  template <typename T> friend class Future;
};

template <typename T> class Future
{
public:
  Future() {}
  ~Future() {join();}

  void abort() {future.abort();}
  bool isAborting() const {return future.isAborting();}
  bool isFinished() const {return future.isFinished();}
  bool isAborted() const {return future.isAborted();}
  void join() {future.join();}

  operator const T&() const {return result;}

  template <typename P, typename A> void start(T (*func)(P), const A& a)
  {
    future.startProc((void (*)(void*))&proc< typename Call<T>::template Args1<P, A> >, new typename Call<T>::template Args1<P, A>(func, a, this));
  }

private:
  Future<void> future;
  T result;

private:
  template <class A> static void proc(A* a);

  Future(const Future&);
  Future& operator=(const Future&);
};


template <class A> void Future<void>::proc(A* a)
{
  a->call();
  ((Future<void>*)a->z)->set();
  delete a;
}

template<typename T> template <class A> void Future<T>::proc(A* a)
{
  ((Future<T>*)a->z)->result = a->call();
  ((Future<T>*)a->z)->future.set();
  delete a;
}
