
#include <nstd/Emitter.h>
#include <nstd/Receiver.h>

Emitter::~Emitter()
{
  for(Map<void*, List<Slot> >::Iterator i = connections.begin(); i != connections.end(); ++i)
  {
    void* signal = i.key();
    const List<Slot>& slots = *i;
    for(List<Slot>::Iterator i = slots.begin(); i != slots.end(); ++i)
    {
      Slot& slotData = *i;
      Map<Emitter*, List<Receiver::Signal> >::Iterator it = slotData.receiver->connections.find(this);
      if(it != slotData.receiver->connections.end())
      {
        List<Receiver::Signal>& signals = *it;
        for(List<Receiver::Signal>::Iterator i = signals.begin(); i != signals.end(); ++i)
          if(i->signal == signal && i->slot == slotData.slot)
          {
            signals.remove(i);
            if(signals.isEmpty())
              slotData.receiver->connections.remove(it);
            break;
          }
      }
    }
  }
}

List<Emitter::Slot>::Iterator Emitter::findSlots(void* signal, List<Slot>::Iterator& start)
{
  Map<void*, List<Slot> >::Iterator it = connections.find(*(void**)&signal);
  if(it == connections.end())
    return start = connections.end()->end();
  return start = it->begin(), it->end();
}
