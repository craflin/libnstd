
#pragma once

#include <nstd/Map.h>
#include <nstd/List.h>

#include <nstd/Console.h>

class Event
{
public:
  class Source;
  class Listener;

private:
  struct MemberFuncPtr
  {
    void (MemberFuncPtr::*ptr)();
    template <typename M> MemberFuncPtr(M member) {Memory::copy(&ptr, &member, sizeof(ptr));}
    bool operator==(const MemberFuncPtr& other) const {return ptr == other.ptr;}
    bool operator>(const MemberFuncPtr& other) const {return Memory::compare(&ptr, &other.ptr, sizeof(ptr)) > 0;}
    bool operator<(const MemberFuncPtr& other) const {return Memory::compare(&ptr, &other.ptr, sizeof(ptr)) < 0;}
    MemberFuncPtr() {}
  };

  template <class X> struct MemberFuncPtr0 {void (X::*ptr)();};
  template <class X, typename A> struct MemberFuncPtr1 {void (X::*ptr)(A);};
  template <class X, typename A, typename B> struct MemberFuncPtr2 {void (X::*ptr)(A, B);};
  template <class X, typename A, typename B, typename C> struct MemberFuncPtr3 {void (X::*ptr)(A, B, C);};
  template <class X, typename A, typename B, typename C, typename D> struct MemberFuncPtr4 {void (X::*ptr)(A, B, C, D);};
  template <class X, typename A, typename B, typename C, typename D, typename E> struct MemberFuncPtr5 {void (X::*ptr)(A, B, C, D, E);};
  template <class X, typename A, typename B, typename C, typename D, typename E, typename F> struct MemberFuncPtr6 {void (X::*ptr)(A, B, C, D, E, F);};
  template <class X, typename A, typename B, typename C, typename D, typename E, typename F, typename G> struct MemberFuncPtr7 {void (X::*ptr)(A, B, C, D, E, F, G);};
  template <class X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H> struct MemberFuncPtr8 {void (X::*ptr)(A, B, C, D, E, F, G, H);};

public:
  class Source
  {
  public:
    ~Source();

  protected:
    template<class X> void emit(void (X::*signal)()) {SignalActivation activation(this, signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected)
      (((X*)i->object)->*((MemberFuncPtr0<X>*)&i->slot))(); if(activation.invalidated) return;}}
    template<class X, typename A> void emit(void (X::*signal)(A), A arg0)
    {
      SignalActivation activation(this, signal);
      for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i)
      {
        if(i->state == Slot::connected)
          (((X*)i->object)->*((MemberFuncPtr1<X, A>*)&i->slot)->ptr)(arg0);
        if(activation.invalidated)
          return;
      }
    }

    template<class X, typename A, typename B> void emit(void (X::*signal)(A, B), A arg0, B arg1){SignalActivation activation(this, signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected) 
      (((X*)i->object)->*((MemberFuncPtr2<X, A, B>*)&i->slot)->ptr)(arg0, arg1); if(activation.invalidated) return;}}
    template<class X, typename A, typename B, typename C> void emit(void (X::*signal)(A, B, C), A arg0, B arg1, C arg2) {SignalActivation activation(this, signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected)
      (((X*)i->object)->*((MemberFuncPtr3<X, A, B, C>*)&i->slot)->ptr)(arg0, arg1, arg2); if(activation.invalidated) return;}}
    template<class X, typename A, typename B, typename C, typename D> void emit(void (X::*signal)(A, B, C, D), A arg0, B arg1, C arg2, D arg3) {SignalActivation activation(this, signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected)
      (((X*)i->object)->*((MemberFuncPtr4<X, A, B, C, D>*)&i->slot)->ptr)(arg0, arg1, arg2, arg3); if(activation.invalidated) return;}}
    template<class X, typename A, typename B, typename C, typename D, typename E> void emit(void (X::*signal)(A, B, C, D, E), A arg0, B arg1, C arg2, D arg3, E arg4) {SignalActivation activation(this, signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected)
      (((X*)i->object)->*((MemberFuncPtr5<X, A, B, C, D, E>*)&i->slot)->ptr)(arg0, arg1, arg2, arg3, arg4); if(activation.invalidated) return;}}
    template<class X, typename A, typename B, typename C, typename D, typename E, typename F> void emit(void (X::*signal)(A, B, C, D, E, F), A arg0, B arg1, C arg2, D arg3, E arg4, F arg5) {SignalActivation activation(this, signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected)
      (((X*)i->object)->*((MemberFuncPtr6<X, A, B, C, D, E, F>*)&i->slot)->ptr)(arg0, arg1, arg2, arg3, arg4, arg5); if(activation.invalidated) return;}}
    template<class X, typename A, typename B, typename C, typename D, typename E, typename F, typename G> void emit(void (X::*signal)(A, B, C, D, E, F, G), A arg0, B arg1, C arg2, D arg3, E arg4, F arg5, G arg6) {SignalActivation activation(this, signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected)
      (((X*)i->object)->*((MemberFuncPtr7<X, A, B, C, D, E, F, G>*)&i->slot)->ptr)(arg0, arg1, arg2, arg3, arg4, arg5, arg6); if(activation.invalidated) return;}}
    template<class X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H> void emit(void (X::*signal)(A, B, C, D, E, F, G, H), A arg0, B arg1, C arg2, D arg3, E arg4, F arg5, G arg6, H arg7) {SignalActivation activation(this, signal); for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i) {if(i->state == Slot::connected)
      (((X*)i->object)->*((MemberFuncPtr8<X, A, B, C, D, E, F, G, H>*)&i->slot)->ptr)(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7); if(activation.invalidated) return;}}

  private:
    struct Slot
    {
      Listener* receiver;
      void* object;
      MemberFuncPtr slot;
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
      SignalActivation(Source* emitter, const MemberFuncPtr& signal);
      ~SignalActivation();
    };

  private:
    Map<MemberFuncPtr, SignalData> signalData;

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
      MemberFuncPtr signal;
      MemberFuncPtr slot;
    };

  private:
    Map<Source*, List<Signal> > slotData;

  private:
    friend class Event;
  };

public:

  /*

  template <class X, typename A> struct MemberFuncPtr1
  {
    static void call(X* x, const MemberFuncPtr& ptr, A a)
    {
       void (X::*ptr)(A);
       ptr = ptr.ptr;
       (x->*ptr)(a);
    }
  };

  template <class X, typename A> struct MemberFuncId
  {
  };
  */

  template<class X, class Y, class V, class W> static void connect(V* src, void (X::*signal)(), W* dest, void (Y::*slot)()) {X* x = src; Y* y = dest; connect(x, signal, y, y, slot);}
  template<class X, class Y, class V, class W, typename A> static void connect(V* src, void (X::*signal)(A), W* dest, void (Y::*slot)(A)) {X* x = src; Y* y = dest; connect(x, signal, y, y, slot);}
  template<class X, class Y, class V, class W, typename A, typename B> static void connect(V* src, void (X::*signal)(A, B), W* dest, void (Y::*slot)(A, B)) {X* x = src; Y* y = dest; connect(x, signal, y, y, slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C> static void connect(V* src, void (X::*signal)(A, B, C), W* dest, void (Y::*slot)(A, B, C)) {X* x = src; Y* y = dest; connect(x, signal, y, y, slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D> static void connect(V* src, void (X::*signal)(A, B, C, D), W* dest, void (Y::*slot)(A, B, C, D)) {X* x = src; Y* y = dest; connect(x, signal, y, y, slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E> static void connect(V* src, void (X::*signal)(A, B, C, D, E), W* dest, void (Y::*slot)(A, B, C, D, E)) {X* x = src; Y* y = dest; connect(x, signal, y, y, slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E, typename F> static void connect(V* src, void (X::*signal)(A, B, C, D, E, F), W* dest, void (Y::*slot)(A, B, C, D, E, F)) {X* x = src; Y* y = dest; connect(x, signal, y, y, slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E, typename F, typename G> static void connect(V* src, void (X::*signal)(A, B, C, D, E, F, G), W* dest, void (Y::*slot)(A, B, C, D, E, F, G)) {X* x = src; Y* y = dest; connect(x, signal, y, y, slot);}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H> static void connect(V* src, void (X::*signal)(A, B, C, D, E, F, G, H), W* dest, void (Y::*slot)(A, B, C, D, E, F, G, H)) {X* x = src; Y* y = dest; connect(x, signal, y, y, slot);}

  template<class X, class Y, class V, class W> static void disconnect(V* src, void (X::*signal)(), W* dest, void (Y::*slot)()) {disconnect(src, MemberFuncPtr(signal), dest, MemberFuncPtr(slot));}
  template<class X, class Y, class V, class W, typename A> static void disconnect(V* src, void (X::*signal)(A), W* dest, void (Y::*slot)(A)) {disconnect(src, MemberFuncPtr(signal), dest, MemberFuncPtr(slot));}
  template<class X, class Y, class V, class W, typename A, typename B> static void disconnect(V* src, void (X::*signal)(A, B), W* dest, void (Y::*slot)(A, B)) {disconnect(src, MemberFuncPtr(signal), dest, MemberFuncPtr(slot));}
  template<class X, class Y, class V, class W, typename A, typename B, typename C> static void disconnect(V* src, void (X::*signal)(A, B, C), W* dest, void (Y::*slot)(A, B, C)) {disconnect(src, MemberFuncPtr(signal), dest, MemberFuncPtr(slot));}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D> static void disconnect(V* src, void (X::*signal)(A, B, C, D), W* dest, void (Y::*slot)(A, B, C, D)) {disconnect(src, MemberFuncPtr(signal), dest, MemberFuncPtr(slot));}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E> static void disconnect(V* src, void (X::*signal)(A, B, C, D, E), W* dest, void (Y::*slot)(A, B, C, D, E)) {disconnect(src, MemberFuncPtr(signal), dest, MemberFuncPtr(slot));}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E, typename F> static void disconnect(V* src, void (X::*signal)(A, B, C, D, E, F), W* dest, void (Y::*slot)(A, B, C, D, E, F)) {disconnect(src, MemberFuncPtr(signal), dest, MemberFuncPtr(slot));}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E, typename F, typename G> static void disconnect(V* src, void (X::*signal)(A, B, C, D, E, F, G), W* dest, void (Y::*slot)(A, B, C, D, E, F, G)) {disconnect(src, MemberFuncPtr(signal), dest, MemberFuncPtr(slot));}
  template<class X, class Y, class V, class W, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H> static void disconnect(V* src, void (X::*signal)(A, B, C, D, E, F, G, H), W* dest, void (Y::*slot)(A, B, C, D, E, F, G, H)) {disconnect(src, MemberFuncPtr(signal), dest, MemberFuncPtr(slot));}

private:
  static void connect(Source* emitter, const MemberFuncPtr& signal, Listener* receiver, void* object, const MemberFuncPtr& slot);
  static void disconnect(Source* emitter, const MemberFuncPtr& signal, Listener* receiver, const MemberFuncPtr& slot);
};
