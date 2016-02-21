
#pragma once

#include <nstd/Map.h>
#include <nstd/List.h>

class Receiver;

class Emitter
{
public:
  ~Emitter();

protected:
  template<class X, typename A> void emit(void (X::*signal)(A), A arg0) {for(List<Slot>::Iterator i, end = findSlots(*(void**)&signal, i); i != end; ++i) *(void**)&signal = i->slot, (((X*)i->receiver)->*signal)(arg0);}
  template<class X, typename A, typename B> void emit(void (X::*signal)(A, B), A arg0, B arg1) {for(List<Slot>::Iterator i, end = findSlots(*(void**)&signal, i); i != end; ++i) *(void**)&signal = i->slot, (((X*)i->receiver)->*signal)(arg0, arg1);}

private:
  struct Slot
  {
    Receiver* receiver;
    void* slot;
  };

private:
  Map<void*, List<Slot> > connections;

private:
  List<Slot>::Iterator findSlots(void* signal, List<Slot>::Iterator& start);

private:
  friend class Receiver;
};
