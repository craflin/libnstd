
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
    void selfDelete(String arg)
    {
      delete this;
    }
  };

  class A
  {
    int a;
  };

  class MyReceiver3 : public MyReceiver, public A
  {
  public:
    MyReceiver3* check3;

  public: // slots
    void mySlot3(String arg)
    {
      ASSERT(this == check);
    }
  };

  class MyReceiver4 : public A, public MyReceiver
  {
  public:
    MyReceiver4* check4;

  public: // slots
    void mySlot4(String arg)
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
  MyReceiver3 receiver3;
  receiver3.check = &receiver3;
  receiver3.check3 = &receiver3;
  MyReceiver4 receiver4;
  receiver4.check = &receiver4;
  receiver4.check4 = &receiver4;

  Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
  Receiver::disconnect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
  Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
  Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
  emitter.emitMySignal("test");
  emitter.emitMySignal2("test", 123);

  Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver3::mySlot3);
  Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver3::mySlot);
  Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver4::mySlot4);
  Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver4::mySlot);
  emitter.emitMySignal("test");
  emitter.emitMySignal2("test", 123);
  Receiver::disconnect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver3::mySlot3);
  Receiver::disconnect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver3::mySlot);
  Receiver::disconnect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver4::mySlot4);
  Receiver::disconnect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver4::mySlot);

  MyReceiver* receiver2 = new MyReceiver;
  Receiver::connect(&emitter, &MyEmitter::mySignal, receiver2, &MyReceiver::mySlot);
  delete receiver2;

  MyEmitter* emitter2 = new MyEmitter;
  Receiver::connect(emitter2, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
  delete emitter2;

  emitter.emitMySignal("test2");

  MyReceiver* receiverSelfDelete = new MyReceiver;
  Receiver::connect(&emitter, &MyEmitter::mySignal, receiverSelfDelete, &MyReceiver::selfDelete);
  emitter.emitMySignal("test2");
}
