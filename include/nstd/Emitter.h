
#pragma once

#include <nstd/Map.h>
#include <nstd/List.h>

class Receiver;

class Emitter
{
public:
  ~Emitter();

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
    Receiver* receiver;
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
    SignalActivation(Emitter* emitter, void* signal) : invalidated(false)
    {
      Map<void*, SignalData>::Iterator it = emitter->signalData.find(*(void**)&signal);
      if(it == emitter->signalData.end())
      {
        data = 0;
        next = 0;
      }
      else
      {
        data = &*it;
        next = data->activation;
        data->activation = this;
        begin = data->slots.begin();
        end = data->slots.end();
      }
    }
    ~SignalActivation()
    {
      if(!invalidated)
      {
        if(data && !(data->activation = next) && data->dirty)
          for(List<Slot>::Iterator i = data->slots.begin(), end = data->slots.end(); i != end;)
            switch(i->state)
            {
            case Slot::disconnected:
              i = data->slots.remove(i);
              break;
            case Slot::connecting:
              i->state = Slot::connected;
            default:
              ++i;
            }
      }
      else if(next)
        next->invalidated = true;
    }
  };

private:
  Map<void*, SignalData> signalData;

private:
  friend class Receiver;
};
