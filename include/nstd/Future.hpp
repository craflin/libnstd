
#pragma once

#include <nstd/Signal.hpp>
#include <nstd/Call.hpp>

template <typename A>
class Future;

template <>
class Future<void>
{
public:
  Future() : _aborting(false), _state(idleState), _joinable(false) {}
  ~Future() { join(); }

public:
  void abort() { _aborting = true; }
  bool isAborting() const { return _aborting; }
  bool isFinished() const { return _state == finishedState; }
  bool isAborted() const { return _state == abortedState; }
  void join()
  {
    if (_joinable)
    {
      _sig.wait();
      _sig.reset();
      _joinable = false;
    }
  }

  void start(void (*func)()) { startProc((void (*)(void *)) & proc<Call<void>::Args0>, new Call<void>::Args0(func, this)); }
  template <typename D, typename P>
  void start(void (*func)(D), const P &p) { startProc((void (*)(void *)) & proc<typename Call<void>::template Args1<D, P> >, new typename Call<void>::template Args1<D, P>(func, p, this)); }
  template <typename D, typename E, typename P, typename Q>
  void start(void (*func)(D, E), const P &p, const Q &q) { startProc((void (*)(void *)) & proc<typename Call<void>::template Args2<D, E, P, Q> >, new typename Call<void>::template Args2<D, E, P, Q>(func, p, q, this)); }
  template <typename D, typename E, typename F, typename P, typename Q, typename R>
  void start(void (*func)(D, E, F), const P &p, const Q &q, const R &r) { startProc((void (*)(void *)) & proc<typename Call<void>::template Args3<D, E, F, P, Q, R> >, new typename Call<void>::template Args3<D, E, F, P, Q, R>(func, p, q, r, this)); }
  template <typename D, typename E, typename F, typename G, typename P, typename Q, typename R, typename S>
  void start(void (*func)(D, E, F, G), const P &p, const Q &q, const R &r, const S &s) { startProc((void (*)(void *)) & proc<typename Call<void>::template Args4<D, E, F, G, P, Q, R, S> >, new typename Call<void>::template Args4<D, E, F, G, P, Q, R, S>(func, p, q, r, s, this)); }
  template <typename D, typename E, typename F, typename G, typename H, typename P, typename Q, typename R, typename S, typename T>
  void start(void (*func)(D, E, F, G, H), const P &p, const Q &q, const R &r, const S &s, const T &t) { startProc((void (*)(void *)) & proc<typename Call<void>::template Args5<D, E, F, G, H, P, Q, R, S, T> >, new typename Call<void>::template Args5<D, E, F, G, H, P, Q, R, S, T>(func, p, q, r, s, t, this)); }

  template <class C>
  void start(C &c, void (C::*func)()) { startProc((void (*)(void *)) & proc<typename Call<void>::template Member<C>::Args0>, new typename Call<void>::template Member<C>::Args0(c, func, this)); }
  template <class C, typename D, typename P>
  void start(C &c, void (C::*func)(D), const P &p) { startProc((void (*)(void *)) & proc<typename Call<void>::template Member<C>::template Args1<D, P> >, new typename Call<void>::template Member<C>::template Args1<D, P>(c, func, p, this)); }
  template <class C, typename D, typename E, typename P, typename Q>
  void start(C &c, void (C::*func)(D, E), const P &p, const Q &q) { startProc((void (*)(void *)) & proc<typename Call<void>::template Member<C>::template Args2<D, E, P, Q> >, new typename Call<void>::template Member<C>::template Args2<D, E, P, Q>(c, func, p, q, this)); }
  template <class C, typename D, typename E, typename F, typename P, typename Q, typename R>
  void start(C &c, void (C::*func)(D, E, F), const P &p, const Q &q, const R &r) { startProc((void (*)(void *)) & proc<typename Call<void>::template Member<C>::template Args3<D, E, F, P, Q, R> >, new typename Call<void>::template Member<C>::template Args3<D, E, F, P, Q, R>(c, func, p, q, r, this)); }
  template <class C, typename D, typename E, typename F, typename G, typename P, typename Q, typename R, typename S>
  void start(C &c, void (C::*func)(D, E, F, G), const P &p, const Q &q, const R &r, const S &s) { startProc((void (*)(void *)) & proc<typename Call<void>::template Member<C>::template Args4<D, E, F, G, P, Q, R, S> >, new typename Call<void>::template Member<C>::template Args4<D, E, F, G, P, Q, R, S>(c, func, p, q, r, s, this)); }

private:
  enum State
  {
    idleState,
    runningState,
    finishedState,
    abortedState,
  };

private:
  Signal _sig;
  volatile bool _aborting;
  volatile int _state;
  bool _joinable;

  void set();

  void startProc(void (*proc)(void *), void *param);

private:
  template <class A>
  static void proc(A *a);

  Future(const Future &);
  Future &operator=(const Future &);

  class Private;
  template <typename T>
  friend class Future;
};

template <typename A>
class Future
{
public:
  Future() {}
  ~Future() { join(); }

  void abort() { future.abort(); }
  bool isAborting() const { return future.isAborting(); }
  bool isFinished() const { return future.isFinished(); }
  bool isAborted() const { return future.isAborted(); }
  void join() { future.join(); }

  operator const A &() const
  {
    future.join();
    return result;
  }

  void start(A (*func)()) { future.startProc((void (*)(void *)) & proc<typename Call<A>::Args0>, new typename Call<A>::Args0(func, this)); }
  template <typename D, typename P>
  void start(A (*func)(D), const P &p) { future.startProc((void (*)(void *)) & proc<typename Call<A>::template Args1<D, P> >, new typename Call<A>::template Args1<D, P>(func, p, this)); }
  template <typename D, typename E, typename P, typename Q>
  void start(A (*func)(D, E), const P &p, const Q &q) { future.startProc((void (*)(void *)) & proc<typename Call<A>::template Args2<D, E, P, Q> >, new typename Call<A>::template Args2<D, E, P, Q>(func, p, q, this)); }
  template <typename D, typename E, typename F, typename P, typename Q, typename R>
  void start(A (*func)(D, E, F), const P &p, const Q &q, const R &r) { future.startProc((void (*)(void *)) & proc<typename Call<A>::template Args3<D, E, F, P, Q, R> >, new typename Call<A>::template Args3<D, E, F, P, Q, R>(func, p, q, r, this)); }
  template <typename D, typename E, typename F, typename G, typename P, typename Q, typename R, typename S>
  void start(A (*func)(D, E, F, G), const P &p, const Q &q, const R &r, const S &s) { future.startProc((void (*)(void *)) & proc<typename Call<A>::template Args4<D, E, F, G, P, Q, R, S> >, new typename Call<A>::template Args4<D, E, F, G, P, Q, R, S>(func, p, q, r, s, this)); }
  template <typename D, typename E, typename F, typename G, typename H, typename P, typename Q, typename R, typename S, typename T>
  void start(A (*func)(D, E, F, G, H), const P &p, const Q &q, const R &r, const S &s, const T &t) { future.startProc((void (*)(void *)) & proc<typename Call<A>::template Args5<D, E, F, G, H, P, Q, R, S, T> >, new typename Call<A>::template Args5<D, E, F, G, H, P, Q, R, S, T>(func, p, q, r, s, t, this)); }

  template <class C>
  void start(C &c, A (C::*func)()) { future.startProc((void (*)(void *)) & proc<typename Call<A>::template Member<C>::Args0>, new typename Call<A>::template Member<C>::Args0(c, func, this)); }
  template <class C, typename D, typename P>
  void start(C &c, A (C::*func)(D), const P &p) { future.startProc((void (*)(void *)) & proc<typename Call<A>::template Member<C>::template Args1<D, P> >, new typename Call<A>::template Member<C>::template Args1<D, P>(c, func, p, this)); }
  template <class C, typename D, typename E, typename P, typename Q>
  void start(C &c, A (C::*func)(D, E), const P &p, const Q &q) { future.startProc((void (*)(void *)) & proc<typename Call<A>::template Member<C>::template Args2<D, E, P, Q> >, new typename Call<A>::template Member<C>::template Args2<D, E, P, Q>(c, func, p, q, this)); }
  template <class C, typename D, typename E, typename F, typename P, typename Q, typename R>
  void start(C &c, A (C::*func)(D, E, F), const P &p, const Q &q, const R &r) { future.startProc((void (*)(void *)) & proc<typename Call<A>::template Member<C>::template Args3<D, E, F, P, Q, R> >, new typename Call<A>::template Member<C>::template Args3<D, E, F, P, Q, R>(c, func, p, q, r, this)); }
  template <class C, typename D, typename E, typename F, typename G, typename P, typename Q, typename R, typename S>
  void start(C &c, A (C::*func)(D, E, F, G), const P &p, const Q &q, const R &r, const S &s) { future.startProc((void (*)(void *)) & proc<typename Call<A>::template Member<C>::template Args4<D, E, F, G, P, Q, R, S> >, new typename Call<A>::template Member<C>::template Args4<D, E, F, G, P, Q, R, S>(c, func, p, q, r, s, this)); }

private:
  mutable Future<void> future;
  A result;

private:
  template <class B>
  static void proc(B *b);

  Future(const Future &);
  Future &operator=(const Future &);
};

template <class A>
void Future<void>::proc(A *a)
{
  a->call();
  ((Future<void> *)a->z)->set();
  delete a;
}

template <typename A>
template <class B>
void Future<A>::proc(B *b)
{
  ((Future<A> *)b->z)->result = b->call();
  ((Future<A> *)b->z)->future.set();
  delete b;
}
