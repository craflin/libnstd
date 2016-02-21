
#include <nstd/Emitter.h>
#include <nstd/Receiver.h>

Receiver::~Receiver()
{
  for(Map<Emitter*, List<Signal> >::Iterator i = connections.begin(); i != connections.end(); ++i)
  {
    Emitter* emitter = i.key();
    const List<Signal>& signals = *i;
    for(List<Signal>::Iterator i = signals.begin(); i != signals.end(); ++i)
    {
      Signal& signalData = *i;
      Map<void*, List<Emitter::Slot> >::Iterator it = emitter->connections.find(signalData.signal);
      if(it != emitter->connections.end())
      {
        List<Emitter::Slot>& slots = *it;
        for(List<Emitter::Slot>::Iterator i = slots.begin(); i != slots.end(); ++i)
          if(i->receiver == this && i->slot == signalData.slot)
          {
            slots.remove(i);
            if(slots.isEmpty())
              emitter->connections.remove(it);
            break;
          }
      }
    }
  }
}

void Receiver::connect(Emitter* emitter, void* signal, Receiver* receiver, void* slot)
{
  Map<void*, List<Emitter::Slot> >::Iterator it = emitter->connections.find(*(void**)&signal);
  Emitter::Slot& slotData = (it == emitter->connections.end() ? emitter->connections.insert(signal, List<Emitter::Slot>()) : it)->append(Emitter::Slot());
  slotData.receiver = receiver;
  slotData.slot = slot;

  Map<Emitter*, List<Receiver::Signal> >::Iterator it2 = receiver->connections.find(emitter);
  Receiver::Signal& signalData = (it2 == receiver->connections.end() ? receiver->connections.insert(emitter, List<Receiver::Signal>()) : it2)->append(Receiver::Signal());
  signalData.signal = signal;
  signalData.slot = slot;
}

void Receiver::disconnect(Emitter* emitter, void* signal, Receiver* receiver, void* slot)
{
  Map<void*, List<Emitter::Slot> >::Iterator it = emitter->connections.find(signal);
  if(it == emitter->connections.end())
    return;
  List<Emitter::Slot>& slots = *it;
  for(List<Emitter::Slot>::Iterator i = slots.begin(); i != slots.end(); ++i)
    if(i->receiver == receiver && i->slot == slot)
    {
      slots.remove(i);
      if(slots.isEmpty())
        emitter->connections.remove(it);
      break;
    }

  Map<Emitter*, List<Receiver::Signal> >::Iterator it2 = receiver->connections.find(emitter);
  List<Receiver::Signal>& signals = *it2;
  for(List<Receiver::Signal>::Iterator i = signals.begin(); i != signals.end(); ++i)
    if(i->signal == signal && i->slot == slot)
    {
      signals.remove(i);
      if(signals.isEmpty())
        receiver->connections.remove(it2);
      break;
    }
}
