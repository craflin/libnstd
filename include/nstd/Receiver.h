
#pragma once

#include <nstd/Map.h>
#include <nstd/List.h>

class Emitter;

class Receiver
{
public:
  ~Receiver();

  template<class X, class Y, typename A> void connect(X* src, void (X::*signal)(A), void (Y::*slot)(A)) {connect(src, *(void**)&signal, this, *(void**)&slot);}
  template<class X, class Y, typename A, typename B> void connect(X* src, void (X::*signal)(A, B), void (Y::*slot)(A, B)) {connect(src, *(void**)&signal, this, *(void**)&slot);}

  template<class X, class Y, typename A> void disconnect(X* src, void (X::*signal)(A), void (Y::*slot)(A)) {disconnect(src, *(void**)&signal, this, *(void**)&slot);}
  template<class X, class Y, typename A, typename B> void disconnect(X* src, void (X::*signal)(A, B), void (Y::*slot)(A, B)) {disconnect(src, *(void**)&signal, this, *(void**)&slot);}

public:
  template<class X, class Y, typename A> static void connect(X* src, void (X::*signal)(A), Y* dest, void (Y::*slot)(A)) {connect(src, *(void**)&signal, dest, *(void**)&slot);}
  template<class X, class Y, typename A, typename B> static void connect(X* src, void (X::*signal)(A, B), Y* dest, void (Y::*slot)(A, B)) {connect(src, *(void**)&signal, dest, *(void**)&slot);}

  template<class X, class Y, typename A> static void disconnect(X* src, void (X::*signal)(A), Y* dest, void (Y::*slot)(A)) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}
  template<class X, class Y, typename A, typename B> static void disconnect(X* src, void (X::*signal)(A, B), Y* dest, void (Y::*slot)(A, B)) {disconnect(src, *(void**)&signal, dest, *(void**)&slot);}

private:
  struct Signal
  {
    void* signal;
    void* slot;
  };

private:
  Map<Emitter*, List<Signal> > connections;

private:
  static void connect(Emitter* emitter, void* signal, Receiver* receiver, void* slot);
  static void disconnect(Emitter* emitter, void* signal, Receiver* receiver, void* slot);

private:
  friend class Emitter;
};
