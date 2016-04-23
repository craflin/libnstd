
#pragma once

#include <nstd/Map.h>
#include <nstd/List.h>

class Emitter;

class Receiver
{
public:
  ~Receiver();

public:
  template<class X, class Y, class V, class W> static void connect(V* src, void (X::*signal)(), W* dest, void (Y::*slot)()) {X* x = src; Y* y = dest; connect(x, *(void**)&signal, y, y, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A> static void connect(V* src, void (X::*signal)(A), W* dest, void (Y::*slot)(A)) {X* x = src; Y* y = dest; connect(x, *(void**)&signal, y, y, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B> static void connect(V* src, void (X::*signal)(A, B), W* dest, void (Y::*slot)(A, B)) {X* x = src; Y* y = dest; connect(x, *(void**)&signal, y, y, *(void**)&slot);}

  template<class X, class Y, class V, class W> static void disconnect(V* src, void (X::*signal)(), W* dest, void (Y::*slot)()) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A> static void disconnect(V* src, void (X::*signal)(A), W* dest, void (Y::*slot)(A)) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B> static void disconnect(V* src, void (X::*signal)(A, B), W* dest, void (Y::*slot)(A, B)) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}

private:
  struct Signal
  {
    void* signal;
    void* slot;
  };

private:
  Map<Emitter*, List<Signal> > connections;

private:
  static void connect(Emitter* emitter, void* signal, Receiver* receiver, void* object, void* slot);
  static void disconnect(Emitter* emitter, void* signal, Receiver* receiver, void* slot);

private:
  friend class Emitter;
};
