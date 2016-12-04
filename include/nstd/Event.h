
#pragma once

#include <nstd/Map.h>
#include <nstd/List.h>

class Event
{
public:
  class Source;
  class Listener;

  class Source
  {
  public:
    ~Source();

  protected:
    template<class X> void emit(void (X::*signal)()) {SignalActivation activation(this, *(void**)&signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected) *(void**)&signal = i->slot, (((X*)i->object)->*signal)(); if(activation.invalidated) return;}}
    template<class X, typename A> void emit(void (X::*signal)(A), A arg0) {SignalActivation activation(this, *(void**)&signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected) *(void**)&signal = i->slot, (((X*)i->object)->*signal)(arg0); if(activation.invalidated) return;}}
    template<class X, typename A, typename B> void emit(void (X::*signal)(A, B), A arg0, B arg1) {SignalActivation activation(this, *(void**)&signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected) *(void**)&signal = i->slot, (((X*)i->object)->*signal)(arg0, arg1); if(activation.invalidated) return;}}
    template<class X, typename A, typename B, typename C> void emit(void (X::*signal)(A, B, C), A arg0, B arg1, C arg2) {SignalActivation activation(this, *(void**)&signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected) *(void**)&signal = i->slot, (((X*)i->object)->*signal)(arg0, arg1, arg2); if(activation.invalidated) return;}}
    template<class X, typename A, typename B, typename C, typename D> void emit(void (X::*signal)(A, B, C, D), A arg0, B arg1, C arg2, D arg3) {SignalActivation activation(this, *(void**)&signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected) *(void**)&signal = i->slot, (((X*)i->object)->*signal)(arg0, arg1, arg2, arg3); if(activation.invalidated) return;}}
    template<class X, typename A, typename B, typename C, typename D, typename E> void emit(void (X::*signal)(A, B, C, D, E), A arg0, B arg1, C arg2, D arg3, E arg4) {SignalActivation activation(this, *(void**)&signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected) *(void**)&signal = i->slot, (((X*)i->object)->*signal)(arg0, arg1, arg2, arg3, arg4); if(activation.invalidated) return;}}
    template<class X, typename A, typename B, typename C, typename D, typename E, typename F> void emit(void (X::*signal)(A, B, C, D, E, F), A arg0, B arg1, C arg2, D arg3, E arg4, F arg5) {SignalActivation activation(this, *(void**)&signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected) *(void**)&signal = i->slot, (((X*)i->object)->*signal)(arg0, arg1, arg2, arg3, arg4, arg5); if(activation.invalidated) return;}}
    template<class X, typename A, typename B, typename C, typename D, typename E, typename F, typename G> void emit(void (X::*signal)(A, B, C, D, E, F, G), A arg0, B arg1, C arg2, D arg3, E arg4, F arg5, G arg6) {SignalActivation activation(this, *(void**)&signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected) *(void**)&signal = i->slot, (((X*)i->object)->*signal)(arg0, arg1, arg2, arg3, arg4, arg5, arg6); if(activation.invalidated) return;}}
    template<class X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H> void emit(void (X::*signal)(A, B, C, D, E, F, G, H), A arg0, B arg1, C arg2, D arg3, E arg4, F arg5, G arg6, H arg7) {SignalActivation activation(this, *(void**)&signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected) *(void**)&signal = i->slot, (((X*)i->object)->*signal)(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7); if(activation.invalidated) return;}}

  private:
    struct Slot
    {
      Listener* receiver;
      void* object;
      void* slot;
      enum State
      {
        connected,
        connecting,
        disconnected,
      } state;
    };

    struct SignalActivation;

    struct SignalData
    {
      SignalActivation* activation;
      List<Slot> slots;
      bool dirty;

      SignalData() : activation(0), dirty(false) {}
    };

    struct SignalActivation
    {
      SignalActivation* next;
      bool invalidated;
      SignalData* data;
      List<Slot>::Iterator begin;
      List<Slot>::Iterator end;
      SignalActivation(Source* emitter, void* signal);
      ~SignalActivation();
    };

  private:
    Map<void*, SignalData> signalData;

  private:
    friend class Event;
  };

  class Listener
  {
  public:
    ~Listener();

  private:
    struct Signal
    {
      void* signal;
      void* slot;
    };

  private:
    Map<Source*, List<Signal> > slotData;

  private:
    friend class Event;
  };

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
  static void connect(Source* emitter, void* signal, Listener* receiver, void* object, void* slot);
  static void disconnect(Source* emitter, void* signal, Listener* receiver, void* slot);
};
