
#include <nstd/Event.h>




Event::Source::~Source()
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
      Map<Event::Source*, List<Event::Listener::Signal> >::Iterator it = slotData.receiver->slotData.find(this);
      if(it != slotData.receiver->slotData.end())
      {
        List<Event::Listener::Signal>& signals = *it;
        for(List<Event::Listener::Signal>::Iterator i = signals.begin(); i != signals.end(); ++i)
          if(i->signal == signal && i->slot == slotData.slot)
          {
            signals.remove(i);
            break;
          }
      }
    }
  }
}

Event::Source::SignalActivation::SignalActivation(Event::Source* emitter, void* signal) : invalidated(false)
{
  Console::printf("SignalActivation emitter=%p signal=%p\n", emitter, signal);
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

Event::Source::SignalActivation::~SignalActivation()
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


Event::Listener::~Listener()
{
  for(Map<Event::Source*, List<Signal> >::Iterator i = slotData.begin(); i != slotData.end(); ++i)
  {
    Event::Source* emitter = i.key();
    const List<Signal>& signals = *i;
    for(List<Signal>::Iterator i = signals.begin(); i != signals.end(); ++i)
    {
      Signal& signalData1 = *i;
      Map<void*, Event::Source::SignalData>::Iterator it = emitter->signalData.find(signalData1.signal);
      if(it != emitter->signalData.end())
      {
        Event::Source::SignalData& signalData = *it;
        for(List<Event::Source::Slot>::Iterator i = signalData.slots.begin(); i != signalData.slots.end(); ++i)
          if(i->receiver == this && i->slot == signalData1.slot)
          {
            if(signalData.activation)
            {
              i->state = Event::Source::Slot::disconnected;
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

void Event::connect(Event::Source* emitter, void* signal, Event::Listener* receiver, void* object, void* slot)
{
  Console::printf("connect emitter=%p signal=%p receiver=%p object=%p slot=%p\n", emitter, signal, receiver, object, slot);

  Map<void*, Event::Source::SignalData>::Iterator it = emitter->signalData.find(signal);
  Event::Source::SignalData& signalData = it == emitter->signalData.end() ? *emitter->signalData.insert(signal, Event::Source::SignalData()) : *it;
  Event::Source::Slot& slotData = signalData.slots.append(Event::Source::Slot());
  if(signalData.activation)
  {
    slotData.state = Event::Source::Slot::connecting;
    signalData.dirty = true;
  }
  else
    slotData.state = Event::Source::Slot::connected;
  slotData.receiver = receiver;
  slotData.object = object;
  slotData.slot = slot;

  Map<Event::Source*, List<Event::Listener::Signal> >::Iterator it2 = receiver->slotData.find(emitter);
  Event::Listener::Signal& signalData2 = (it2 == receiver->slotData.end() ? receiver->slotData.insert(emitter, List<Event::Listener::Signal>()) : it2)->append(Event::Listener::Signal());
  signalData2.signal = signal;
  signalData2.slot = slot;
}

void Event::disconnect(Event::Source* emitter, void* signal, Event::Listener* receiver, void* slot)
{
  Map<void*, Event::Source::SignalData>::Iterator it = emitter->signalData.find(signal);
  if(it == emitter->signalData.end())
    return;

  Event::Source::SignalData& signalData = *it;
  for(List<Event::Source::Slot>::Iterator i = signalData.slots.begin(); i != signalData.slots.end(); ++i)
    if(i->receiver == receiver && i->slot == slot)
    {
      if(signalData.activation)
      {
        i->state = Event::Source::Slot::disconnected;
        signalData.dirty = true;
      }
      else
        signalData.slots.remove(i);
      break;
    }

  Map<Event::Source*, List<Event::Listener::Signal> >::Iterator it2 = receiver->slotData.find(emitter);
  List<Event::Listener::Signal>& signals = *it2;
  for(List<Event::Listener::Signal>::Iterator i = signals.begin(); i != signals.end(); ++i)
    if(i->signal == signal && i->slot == slot)
    {
      signals.remove(i);
      break;
    }
}
