
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
      Signal& signalData1 = *i;
      Map<void*, Emitter::SignalData>::Iterator it = emitter->signalData.find(signalData1.signal);
      if(it != emitter->signalData.end())
      {
        Emitter::SignalData& signalData = *it;
        for(List<Emitter::Slot>::Iterator i = signalData.slots.begin(); i != signalData.slots.end(); ++i)
          if(i->receiver == this && i->slot == signalData1.slot)
          {
            if(signalData.activation)
            {
              i->state = Emitter::Slot::disconnected;
              signalData.dirty = true;
            }
            else
            {
              signalData.slots.remove(i);
              if(signalData.slots.isEmpty())
                emitter->signalData.remove(it);
            }
            break;
          }
      }
    }
  }
}

void Receiver::connect(Emitter* emitter, void* signal, Receiver* receiver, void* object, void* slot)
{
  Map<void*, Emitter::SignalData>::Iterator it = emitter->signalData.find(signal);
  Emitter::SignalData& signalData = it == emitter->signalData.end() ? *emitter->signalData.insert(signal, Emitter::SignalData()) : *it;
  Emitter::Slot& slotData = signalData.slots.append(Emitter::Slot());
  slotData.state = signalData.activation ? Emitter::Slot::connecting : Emitter::Slot::connected;
  slotData.receiver = receiver;
  slotData.object = object;
  slotData.slot = slot;

  Map<Emitter*, List<Receiver::Signal> >::Iterator it2 = receiver->connections.find(emitter);
  Receiver::Signal& signalData2 = (it2 == receiver->connections.end() ? receiver->connections.insert(emitter, List<Receiver::Signal>()) : it2)->append(Receiver::Signal());
  signalData2.signal = signal;
  signalData2.slot = slot;
}

void Receiver::disconnect(Emitter* emitter, void* signal, Receiver* receiver, void* slot)
{
  Map<void*, Emitter::SignalData>::Iterator it = emitter->signalData.find(signal);
  if(it == emitter->signalData.end())
    return;

  Emitter::SignalData& signalData = *it;
  for(List<Emitter::Slot>::Iterator i = signalData.slots.begin(); i != signalData.slots.end(); ++i)
    if(i->receiver == receiver && i->slot == slot)
    {
      if(signalData.activation)
      {
        i->state = Emitter::Slot::disconnected;
        signalData.dirty = true;
      }
      else
      {
        signalData.slots.remove(i);
        if(signalData.slots.isEmpty())
          emitter->signalData.remove(it);
      }
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
