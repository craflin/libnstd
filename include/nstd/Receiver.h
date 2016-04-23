
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
  template<class X, class Y, class V, class W, typename A, typename B, typename C> static void connect(V* src, void (X::*signal)(A, B, C), W* dest, void (Y::*slot)(A, B, C)) {X* x = src; Y* y = dest; connect(x, *(void**)&signal, y, y, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D> static void connect(V* src, void (X::*signal)(A, B, C, D), W* dest, void (Y::*slot)(A, B, C, D)) {X* x = src; Y* y = dest; connect(x, *(void**)&signal, y, y, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E> static void connect(V* src, void (X::*signal)(A, B, C, D, E), W* dest, void (Y::*slot)(A, B, C, D, E)) {X* x = src; Y* y = dest; connect(x, *(void**)&signal, y, y, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E, typename F> static void connect(V* src, void (X::*signal)(A, B, C, D, E, F), W* dest, void (Y::*slot)(A, B, C, D, E, F)) {X* x = src; Y* y = dest; connect(x, *(void**)&signal, y, y, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E, typename F, typename G> static void connect(V* src, void (X::*signal)(A, B, C, D, E, F, G), W* dest, void (Y::*slot)(A, B, C, D, E, F, G)) {X* x = src; Y* y = dest; connect(x, *(void**)&signal, y, y, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H> static void connect(V* src, void (X::*signal)(A, B, C, D, E, F, G, H), W* dest, void (Y::*slot)(A, B, C, D, E, F, G, H)) {X* x = src; Y* y = dest; connect(x, *(void**)&signal, y, y, *(void**)&slot);}

  template<class X, class Y, class V, class W> static void disconnect(V* src, void (X::*signal)(), W* dest, void (Y::*slot)()) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A> static void disconnect(V* src, void (X::*signal)(A), W* dest, void (Y::*slot)(A)) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B> static void disconnect(V* src, void (X::*signal)(A, B), W* dest, void (Y::*slot)(A, B)) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C> static void disconnect(V* src, void (X::*signal)(A, B, C), W* dest, void (Y::*slot)(A, B, C)) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D> static void disconnect(V* src, void (X::*signal)(A, B, C, D), W* dest, void (Y::*slot)(A, B, C, D)) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E> static void disconnect(V* src, void (X::*signal)(A, B, C, D, E), W* dest, void (Y::*slot)(A, B, C, D, E)) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E, typename F> static void disconnect(V* src, void (X::*signal)(A, B, C, D, E, F), W* dest, void (Y::*slot)(A, B, C, D, E, F)) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E, typename F, typename G> static void disconnect(V* src, void (X::*signal)(A, B, C, D, E, F, G), W* dest, void (Y::*slot)(A, B, C, D, E, F, G)) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H> static void disconnect(V* src, void (X::*signal)(A, B, C, D, E, F, G, H), W* dest, void (Y::*slot)(A, B, C, D, E, F, G, H)) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}

private:
  struct Signal
  {
    void* signal;
    void* slot;
  };

private:
  Map<Emitter*, List<Signal> > slotData;

private:
  static void connect(Emitter* emitter, void* signal, Receiver* receiver, void* object, void* slot);
  static void disconnect(Emitter* emitter, void* signal, Receiver* receiver, void* slot);

private:
  friend class Emitter;
};
