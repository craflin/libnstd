
#include <nstd/Emitter.h>
#include <nstd/Receiver.h>
#include <nstd/String.h>
#include <nstd/Debug.h>

void testReceiver()
{
  class MyReceiver : public Receiver
  {
  public:
    MyReceiver* check;

  public: // slots
    void mySlot(String arg)
    {
      ASSERT(this == check);
    }
  };

  class MyEmitter : public Emitter
  {
  public:
    void emitMySignal(const String& arg)
    {
      emit(&MyEmitter::mySignal, arg);
    }
    void emitMySignal2(const String& arg0, int arg1)
    {
      emit(&MyEmitter::mySignal2, arg0, arg1);
    }
 
  public: // signals
    void mySignal(String arg) {}
    void mySignal2(String arg, int arg1) {}
  };

  MyEmitter emitter;
  MyReceiver receiver;
  receiver.check = &receiver;
  receiver.connect(&emitter, &MyEmitter::mySignal, &MyReceiver::mySlot);
  receiver.disconnect(&emitter, &MyEmitter::mySignal, &MyReceiver::mySlot);
  receiver.connect(&emitter, &MyEmitter::mySignal, &MyReceiver::mySlot);
  Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
  emitter.emitMySignal("test");
  emitter.emitMySignal2("test", 123);

  MyReceiver* receiver2 = new MyReceiver;
  Receiver::connect(&emitter, &MyEmitter::mySignal, receiver2, &MyReceiver::mySlot);
  delete receiver2;

  MyEmitter* emitter2 = new MyEmitter;
  Receiver::connect(emitter2, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
  delete emitter2;

  emitter.emitMySignal("test2");
}
