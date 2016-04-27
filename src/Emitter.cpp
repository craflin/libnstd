
#include <nstd/Emitter.h>
#include <nstd/Receiver.h>

Emitter::~Emitter()
{
  for(Map<void*, SignalData>::Iterator i = signalData.begin(); i != signalData.end(); ++i)
  {
    void* signal = i.key();
    SignalData& data = *i;
    if(data.activation)
      data.activation->invalidated = true;
    for(List<Slot>::Iterator i = data.slots.begin(), end = data.slots.end(); i != end; ++i)
    {
      Slot& slotData = *i;
      if(slotData.state == Slot::disconnected)
        continue;
      Map<Emitter*, List<Receiver::Signal> >::Iterator it = slotData.receiver->slotData.find(this);
      if(it != slotData.receiver->slotData.end())
      {
        List<Receiver::Signal>& signals = *it;
        for(List<Receiver::Signal>::Iterator i = signals.begin(); i != signals.end(); ++i)
          if(i->signal == signal && i->slot == slotData.slot)
          {
            signals.remove(i);
            break;
          }
      }
    }
  }
}

Emitter::SignalActivation::SignalActivation(Emitter* emitter, void* signal) : invalidated(false)
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

Emitter::SignalActivation::~SignalActivation()
{
  if(!invalidated)
  {
    if(data && !(data->activation = next) && data->dirty)
    {
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
      data->dirty = false;
    }
  }
  else if(next)
    next->invalidated = true;
}
