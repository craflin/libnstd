
#include <nstd/Callback.h>

Callback::Emitter::~Emitter()
{
  for(Map<MemberFuncPtr, SignalData>::Iterator i = signalData.begin(); i != signalData.end(); ++i)
  {
    const MemberFuncPtr& signal = i.key();
    SignalData& data = *i;
    if(data.activation)
      data.activation->invalidated = true;
    for(List<Slot>::Iterator i = data.slots.begin(), end = data.slots.end(); i != end; ++i)
    {
      Slot& slotData = *i;
      if(slotData.state == Slot::disconnected)
        continue;
      Map<Callback::Emitter*, List<Callback::Listener::Signal> >::Iterator it = slotData.receiver->slotData.find(this);
      if(it != slotData.receiver->slotData.end())
      {
        List<Callback::Listener::Signal>& signals = *it;
        for(List<Callback::Listener::Signal>::Iterator i = signals.begin(); i != signals.end(); ++i)
          if(i->signal == signal && i->slot == slotData.slot)
          {
            signals.remove(i);
            break;
          }
      }
    }
  }
}

Callback::Emitter::SignalActivation::SignalActivation(Callback::Emitter* emitter, const MemberFuncPtr& signal) : invalidated(false)
{
  Map<MemberFuncPtr, SignalData>::Iterator it = emitter->signalData.find(signal);
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

Callback::Emitter::SignalActivation::~SignalActivation()
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


Callback::Listener::~Listener()
{
  for(Map<Callback::Emitter*, List<Signal> >::Iterator i = slotData.begin(); i != slotData.end(); ++i)
  {
    Callback::Emitter* emitter = i.key();
    const List<Signal>& signals = *i;
    for(List<Signal>::Iterator i = signals.begin(); i != signals.end(); ++i)
    {
      Signal& signalData1 = *i;
      Map<MemberFuncPtr, Callback::Emitter::SignalData>::Iterator it = emitter->signalData.find(signalData1.signal);
      if(it != emitter->signalData.end())
      {
        Callback::Emitter::SignalData& signalData = *it;
        for(List<Callback::Emitter::Slot>::Iterator i = signalData.slots.begin(); i != signalData.slots.end(); ++i)
          if(i->receiver == this && i->slot == signalData1.slot)
          {
            if(signalData.activation)
            {
              i->state = Callback::Emitter::Slot::disconnected;
              signalData.dirty = true;
            }
            else
              signalData.slots.remove(i);
            break;
          }
      }
    }
  }
}

void Callback::connect(Callback::Emitter* emitter, const MemberFuncPtr& signal, Callback::Listener* receiver, void* object, const MemberFuncPtr& slot)
{
  Map<MemberFuncPtr, Callback::Emitter::SignalData>::Iterator it = emitter->signalData.find(signal);
  Callback::Emitter::SignalData& signalData = it == emitter->signalData.end() ? *emitter->signalData.insert(signal, Callback::Emitter::SignalData()) : *it;
  Callback::Emitter::Slot& slotData = signalData.slots.append(Callback::Emitter::Slot());
  if(signalData.activation)
  {
    slotData.state = Callback::Emitter::Slot::connecting;
    signalData.dirty = true;
  }
  else
    slotData.state = Callback::Emitter::Slot::connected;
  slotData.receiver = receiver;
  slotData.object = object;
  slotData.slot = slot;

  Map<Callback::Emitter*, List<Callback::Listener::Signal> >::Iterator it2 = receiver->slotData.find(emitter);
  Callback::Listener::Signal& signalData2 = (it2 == receiver->slotData.end() ? receiver->slotData.insert(emitter, List<Callback::Listener::Signal>()) : it2)->append(Callback::Listener::Signal());
  signalData2.signal = signal;
  signalData2.slot = slot;
}

void Callback::disconnect(Callback::Emitter* emitter, const MemberFuncPtr& signal, Callback::Listener* receiver, const MemberFuncPtr& slot)
{
  Map<MemberFuncPtr, Callback::Emitter::SignalData>::Iterator it = emitter->signalData.find(signal);
  if(it == emitter->signalData.end())
    return;

  Callback::Emitter::SignalData& signalData = *it;
  for(List<Callback::Emitter::Slot>::Iterator i = signalData.slots.begin(); i != signalData.slots.end(); ++i)
    if(i->receiver == receiver && i->slot == slot)
    {
      if(signalData.activation)
      {
        i->state = Callback::Emitter::Slot::disconnected;
        signalData.dirty = true;
      }
      else
        signalData.slots.remove(i);
      break;
    }

  Map<Callback::Emitter*, List<Callback::Listener::Signal> >::Iterator it2 = receiver->slotData.find(emitter);
  List<Callback::Listener::Signal>& signals = *it2;
  for(List<Callback::Listener::Signal>::Iterator i = signals.begin(); i != signals.end(); ++i)
    if(i->signal == signal && i->slot == slot)
    {
      signals.remove(i);
      break;
    }
}
