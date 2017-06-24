
#pragma once

#include <nstd/Signal.h>

template <typename A> struct Call
{
  template <class C> struct Member
  {
    struct Func0
    {
      C& c;
      A (C::*a)();
      A call() {return (c.*a)();}
      Func0(C& c, A (C::*a)()) : c(c), a(a) {}
    };

    template <typename D> struct Func1
    {
      C& c;
      A (C::*a)(D);
      A call(D d) {return (c.*a)(d);}
      Func1(C& c, A (C::*a)(D)) : c(c), a(a) {}
    };

    template <typename D, typename E> struct Func2
    {
      C& c;
      A (C::*a)(D, E);
      A call(D d, E e) {return (c.*a)(d, e);}
      Func2(C& c, A (C::*a)(D, E)) : c(c), a(a) {}
    };

    template <typename D, typename E, typename F> struct Func3
    {
      C& c;
      A (C::*a)(D, E, F);
      A call(D d, E e, F f) {return (c.*a)(d, e, f);}
      Func3(C& c, A (C::*a)(D, E, F)) : c(c), a(a) {}
    };

    template <typename D, typename E, typename F, typename G> struct Func4
    {
      C& c;
      A (C::*a)(D, E, F, G);
      A call(D d, E e, F f, G g) {return (c.*a)(d, e, f, g);}
      Func4(C& c, A (C::*a)(D, E, F, G)) : c(c), a(a) {}
    };

    struct Args0 : public Func0
    {
      void* z;
      A call() {return (Func0::c.*Func0::a)();}
      Args0(C& c, A (C::*a)(), void* z) :  Func0(c, a), z(z) {}
    };

    template <typename D, typename P> struct Args1 : public Func1<D>
    {
      P p;
      void* z;
      A call() {return (Func1<D>::c.*Func1<D>::a)(p);}
      Args1(C& c, A (C::*a)(D), const P& p, void* z) :  Func1<D>(c, a), p(p), z(z) {}
    };

    template <typename D, typename E, typename P, typename Q> struct Args2 : public Func2<D, E>
    {
      P p; Q q;
      void* z;
      A call() {return (Func2<D, E>::c.*Func2<D, E>::a)(p, q);}
      Args2(C& c, A (C::*a)(D, E), const P& p, const Q& q, void* z) :  Func2<D, E>(c, a), p(p), q(q), z(z) {}
    };

    template <typename D, typename E, typename F, typename P, typename Q, typename R> struct Args3 : public Func3<D, E, F>
    {
      P p; Q q; R r;
      void* z;
      A call() {return (Func3<D, E, F>::c.*Func3<D, E, F>::a)(p, q, r);}
      Args3(C& c, A (C::*a)(D, E, F), const P& p, const Q& q, const R& r, void* z) :  Func3<D, E, F>(c, a), p(p), q(q), r(r), z(z) {}
    };

    template <typename D, typename E, typename F, typename G, typename P, typename Q, typename R, typename S> struct Args4 : public Func4<D, E, F, G>
    {
      P p; Q q; R r; S s;
      void* z;
      A call() {return (Func4<D, E, F, G>::c.*Func4<D, E, F, G>::a)(p, q, r, s);}
      Args4(C& c, A (C::*a)(D, E, F, G), const P& p, const Q& q, const R& r, const S& s, void* z) :  Func4<D, E, F, G>(c, a), p(p), q(q), r(r), s(s), z(z) {}
    };
  };

  struct Func0
  {
    A (*a)();
    A call() {return a();}
    Func0(A (*a)()) : a(a) {}
  };

  template <typename D> struct Func1
  {
    A (*a)(D);
    A call(D d) {return a(d);}
    Func1(A (*a)(D)) : a(a) {}
  };

  template <typename D, typename E> struct Func2
  {
    A (*a)(D, E);
    A call(D d, E e) {return a(d, e);}
    Func2(A (*a)(D, E)) : a(a) {}
  };

  template <typename D, typename E, typename F> struct Func3
  {
    A (*a)(D, E, F);
    A call(D d, E e, F f) {return a(d, e, f);}
    Func3(A (*a)(D, E, F)) : a(a) {}
  };

  template <typename D, typename E, typename F, typename G> struct Func4
  {
    A (*a)(D, E, F, G);
    A call(D d, E e, F f, G g) {return a(d, e, f, g);}
    Func4(A (*a)(D, E, F, G)) : a(a) {}
  };

  template <typename D, typename E, typename F, typename G, typename H> struct Func5
  {
    A (*a)(D, E, F, G, H);
    A call(D d, E e, F f, G g, H h) {return a(d, e, f, g, h);}
    Func5(A (*a)(D, E, F, G, H)) : a(a) {}
  };

  struct Args0 : public Func0
  {
    void* z;
    A call() {return Func0::a();}
    Args0(A (*a)(), void* z) : Func0(a), z(z) {}
  };

  template <typename D, typename P> struct Args1 : public Func1<D>
  {
    P p;
    void* z;
    A call() {return Func1<D>::a(p);}
    Args1(A (*a)(D), const P& p, void* z) : Func1<D>(a), p(p), z(z) {}
  };

  template <typename D, typename E, typename P, typename Q> struct Args2 : public Func2<D, E>
  {
    P p; Q q;
    void* z;
    A call() {return Func2<D, E>::a(p, q);}
    Args2(A (*a)(D, E), const P& p, const Q& q, void* z) : Func2<D, E>(a), p(p), q(q), z(z) {}
  };

  template <typename D, typename E, typename F, typename P, typename Q, typename R> struct Args3 : public Func3<D, E, F>
  {
    P p; Q q; R r;
    void* z;
    A call() {return Func3<D, E, F>::a(p, q, r);}
    Args3(A (*a)(D, E, F), const P& p, const Q& q, const R& r, void* z) : Func3<D, E, F>(a), p(p), q(q), r(r), z(z) {}
  };

  template <typename D, typename E, typename F, typename G, typename P, typename Q, typename R, typename S> struct Args4 : public Func4<D, E, F, G>
  {
    P p; Q q; R r; S s;
    void* z;
    A call() {return Func4<D, E, F, G>::a(p, q, r, s);}
    Args4(A (*a)(D, E, F, G), const P& p, const Q& q, const R& r, const S& s, void* z) : Func4<D, E, F, G>(a), p(p), q(q), r(r), s(s), z(z) {}
  };

  template <typename D, typename E, typename F, typename G, typename H, typename P, typename Q, typename R, typename S, typename T> struct Args5 : public Func5<D, E, F, G, H>
  {
    P p; Q q; R r; S s; T t;
    void* z;
    A call() {return Func5<D, E, F, G, H>::a(p, q, r, s, t);}
    Args5(A (*a)(D, E, F, G,H), const P& p, const Q& q, const R& r, const S& s, const T& t, void* z) : Func5<D, E, F, G, H>(a), p(p), q(q), r(r), s(s), t(t), z(z) {}
  };
};

template <typename T> class Future;

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

  void start(void (*func)()) {startProc((void (*)(void*))&proc< Call<void>::Args0 >, new Call<void>::Args0(func, this));}
  template <typename D, typename P> void start(void (*func)(D), const P& p) {startProc((void (*)(void*))&proc< typename Call<void>::template Args1<D, P> >, new typename Call<void>::template Args1<D, P>(func, p, this));}

  template <class C, typename D, typename P> void start(C& c, void (C::*func)(D), const P& p){startProc((void (*)(void*))&proc< typename Call<void>::template Member<C>::template Args1<D, P> >, new typename Call<void>::template Member<C>::template Args1<D, P>(c, func, p, this));}

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

  template <typename D, typename P> void start(T (*func)(D), const P& p) {future.startProc((void (*)(void*))&proc< typename Call<T>::template Args1<D, P> >, new typename Call<T>::template Args1<D, P>(func, p, this));}

  template <class C, typename D, typename P> void start(C& c, T (C::*func)(D), const P& p) {future.startProc((void (*)(void*))&proc< typename Call<T>::template Member<C>::template Args1<D, P> >, new typename Call<T>::template Member<C>::template Args1<D, P>(c, func, p, this));}

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
