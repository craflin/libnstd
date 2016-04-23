
#include <nstd/Emitter.h>
#include <nstd/Receiver.h>
#include <nstd/String.h>
#include <nstd/Debug.h>

void testReceiver()
{
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

  class MyReceiver : public Receiver
  {
  public:
    MyReceiver* check;
    MyEmitter* emitter;

  public: // slots
    virtual void mySlot(String arg)
    {
      ASSERT(this == check);
    }
    void selfDelete(String arg)
    {
      ASSERT(this == check);
      delete this;
    }
    void selfDisconnect(String arg)
    {
      ASSERT(this == check);
      disconnect(emitter, &MyEmitter::mySignal, this, &MyReceiver::selfDisconnect);
    }
    void emitterDelete(String arg)
    {
      ASSERT(this == check);
      disconnect(emitter, &MyEmitter::mySignal, this, &MyReceiver::emitterDelete);
      delete emitter;
    }
  };


  // test simple connect and disconnect
  {
    MyEmitter emitter;
    MyReceiver receiver;
    receiver.check = &receiver;

    Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
    Receiver::disconnect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
    Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
    Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
    emitter.emitMySignal("test");
    emitter.emitMySignal2("test", 123);
  }

  // test slots in derived classes
  {
    class A
    {
      int a;

      virtual void ha(String arg){}
    };

    class MyReceiver3 : public MyReceiver, public A
    {
    public:
      MyReceiver3* check3;
      int mySlotCalled;

      MyReceiver3() : mySlotCalled(0) {}

    public: // slots
      void mySlot3(String arg)
      {
        ASSERT(this == check);
      }

      virtual void mySlot(String arg)
      {
        ++mySlotCalled;
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

    MyEmitter emitter;
    MyReceiver receiver;
    receiver.check = &receiver;
    MyReceiver3 receiver3;
    receiver3.check = &receiver3;
    receiver3.check3 = &receiver3;
    ASSERT((void*)receiver3.check == (void*)receiver3.check3);
    MyReceiver4 receiver4;
    receiver4.check = &receiver4;
    receiver4.check4 = &receiver4;
    ASSERT((void*)receiver4.check != (void*)receiver4.check4);

    Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver3::mySlot3);
    Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver::mySlot);
    Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver3::mySlot);
    Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver4::mySlot4);
    Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver::mySlot);
    Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver4::mySlot);
    emitter.emitMySignal("test");
    emitter.emitMySignal2("test", 123);
    ASSERT(receiver3.mySlotCalled == 2);
    Receiver::disconnect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver3::mySlot3);
    Receiver::disconnect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver::mySlot);
    Receiver::disconnect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver4::mySlot4);
    Receiver::disconnect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver::mySlot);
  }

  // test destructor of Receiver
  {
    MyEmitter emitter;
    MyReceiver receiver;
    receiver.check = &receiver;

    MyReceiver* receiver2 = new MyReceiver;
    Receiver::connect(&emitter, &MyEmitter::mySignal, receiver2, &MyReceiver::mySlot);
    delete receiver2;

    emitter.emitMySignal("test2");
  }

  // test destructor of Emitter
  {
    MyReceiver receiver;
    receiver.check = &receiver;

    MyEmitter* emitter2 = new MyEmitter;
    Receiver::connect(emitter2, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
    delete emitter2;
  }

  // test delete of receiver in slot
  {
    MyEmitter emitter;
    MyReceiver receiver;
    receiver.check = &receiver;
    MyReceiver* receiverSelfDelete = new MyReceiver;
    receiverSelfDelete->check = receiverSelfDelete;
    Receiver::connect(&emitter, &MyEmitter::mySignal, receiverSelfDelete, &MyReceiver::selfDelete);
    Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
    emitter.emitMySignal("test2");
    emitter.emitMySignal("test2");
  }

  // test disconnect of receiver in slot
  {
    MyEmitter emitter;
    MyReceiver receiver1;
    receiver1.check = &receiver1;
    receiver1.emitter = &emitter;
    MyReceiver receiver2;
    receiver2.check = &receiver2;
    Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver1, &MyReceiver::selfDisconnect);
    Receiver::connect(&emitter, &MyEmitter::mySignal, &receiver2, &MyReceiver::mySlot);
    emitter.emitMySignal("test2");
    emitter.emitMySignal("test2");
  }

  // test delete of emitter in slot
  {
    MyEmitter* emitter = new MyEmitter;
    MyEmitter emitter2;
    MyReceiver receiver1;
    receiver1.check = &receiver1;
    receiver1.emitter = emitter;
    MyReceiver receiver2;
    receiver2.check = &receiver2;
    Receiver::connect(emitter, &MyEmitter::mySignal, &receiver1, &MyReceiver::emitterDelete);
    Receiver::connect(emitter, &MyEmitter::mySignal, &receiver2, &MyReceiver::mySlot);
    Receiver::connect(&emitter2, &MyEmitter::mySignal, &receiver2, &MyReceiver::mySlot);
    emitter->emitMySignal("test2");
    emitter2.emitMySignal("test2");
  }
}
