
#pragma once

#include <nstd/Map.h>
#include <nstd/List.h>

class Receiver;

class Emitter
{
public:
  ~Emitter();

protected:
  template<class X> void emit(void (X::*signal)())
  {
    SignalActivation activation(this, *(void**)&signal);
    for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i)
    {
      if(i->state == Slot::connected)
        *(void**)&signal = i->slot, (((X*)i->object)->*signal)();
      if(activation.invalidated)
        return;
    }
  }
  template<class X, typename A> void emit(void (X::*signal)(A), A arg0)
  {
    SignalActivation activation(this, *(void**)&signal);
    for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i)
    {
      if(i->state == Slot::connected)
      *(void**)&signal = i->slot, (((X*)i->object)->*signal)(arg0);
      if(activation.invalidated)
        return;
    }
  }
  template<class X, typename A, typename B> void emit(void (X::*signal)(A, B), A arg0, B arg1)
  {
    SignalActivation activation(this, *(void**)&signal);
    for(List<Slot>::Iterator i = activation.begin; i != activation.end; ++i)
    {
      if(i->state == Slot::connected)
      *(void**)&signal = i->slot, (((X*)i->object)->*signal)(arg0, arg1);
      if(activation.invalidated)
        return;
    }
  }

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

    void cleanup()
    {
      for(List<Slot>::Iterator i = slots.begin(), end = slots.end(); i != end;)
        switch(i->state)
        {
        case Slot::disconnected:
          i = slots.remove(i);
          break;
        case Slot::connecting:
          i->state = Slot::connected;
        default:
          ++i;
        }
      // todo: remove slot data?
      //if(slots.isEmpty())
      //  ??
    }
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
          data->cleanup();
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
