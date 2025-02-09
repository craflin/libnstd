
#include <nstd/Callback.hpp>
#include <nstd/String.hpp>
#include <nstd/Debug.hpp>

class MyEmitter : public Callback::Emitter
{
public:
  virtual ~MyEmitter() {}
  void emitMySignal(const String& arg)
  {
    emit(&MyEmitter::mySignal, arg);
  }

  void emitMySignal0()
  {
    emit(&MyEmitter::mySignal0);
  }

  void emitMySignal2(const String& arg0, int arg1)
  {
    emit(&MyEmitter::mySignal2, arg0, arg1);
  }

  void emitMySignal3()
  {
    emit<MyEmitter, const String&>(&MyEmitter::mySignal3, "test");
  }
 
public: // signals
  virtual void mySignal(String arg) {}
  virtual void mySignal0() {}
  virtual void mySignal2(String arg, int arg1) {}
  virtual void mySignal3(const String& arg) {}
};

class MyReceiver : public Callback::Listener
{
public:
  MyReceiver* check;
  MyEmitter* emitter;
  int calls;

  MyReceiver() : calls(0) {}
  virtual ~MyReceiver() {}

public: // slots
  virtual void mySlot(String arg)
  {
    ASSERT(this == check);
    ++calls;
  }
  void selfDelete(String arg)
  {
    ASSERT(this == check);
    delete this;
  }
  void selfDisconnect(String arg)
  {
    ASSERT(this == check);
    Callback::disconnect(emitter, &MyEmitter::mySignal, this, &MyReceiver::selfDisconnect);
  }
  void emitterDelete(String arg)
  {
    ASSERT(this == check);
    Callback::disconnect(emitter, &MyEmitter::mySignal, this, &MyReceiver::emitterDelete);
    delete emitter;
  }
  void mySlot0()
  {
    ASSERT(this == check);
    ++calls;
  }
};

void testSimpleConnectAndDisconnect()
{ // test simple connect and disconnect
  MyEmitter emitter;
  MyReceiver receiver;
  receiver.check = &receiver;

  Callback::connect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
  Callback::disconnect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
  Callback::connect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
  Callback::connect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
  emitter.emitMySignal("test");
  emitter.emitMySignal2("test", 123);
  ASSERT(receiver.calls == 2);
}

void testCallback()
{
  // test simple connect and disconnect without args
  {
    MyEmitter emitter;
    MyReceiver receiver;
    receiver.check = &receiver;
    Callback::connect(&emitter, &MyEmitter::mySignal0, &receiver, &MyReceiver::mySlot0);
    emitter.emitMySignal0();
    ASSERT(receiver.calls == 1);
  }

  // test signal with const String& argument
  {
    MyEmitter emitter;
    emitter.emitMySignal3();
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
        ++calls;
      }

      virtual void mySlot(String arg)
      {
        ++mySlotCalled;
        ++calls;
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
        ++calls;
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

    Callback::connect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver3::mySlot3);
    Callback::connect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver::mySlot);
    Callback::connect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver3::mySlot);
    Callback::connect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver4::mySlot4);
    Callback::connect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver::mySlot);
    Callback::connect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver4::mySlot);
    emitter.emitMySignal("test");
    emitter.emitMySignal2("test", 123);
    ASSERT(receiver3.mySlotCalled == 2);
    ASSERT(receiver3.calls == 3);
    ASSERT(receiver4.calls == 3);
    Callback::disconnect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver3::mySlot3);
    Callback::disconnect(&emitter, &MyEmitter::mySignal, &receiver3, &MyReceiver::mySlot);
    Callback::disconnect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver4::mySlot4);
    Callback::disconnect(&emitter, &MyEmitter::mySignal, &receiver4, &MyReceiver::mySlot);
  }

  // test destructor of Receiver
  {
    MyEmitter emitter;
    MyReceiver receiver;
    receiver.check = &receiver;

    MyReceiver* receiver2 = new MyReceiver;
    Callback::connect(&emitter, &MyEmitter::mySignal, receiver2, &MyReceiver::mySlot);
    delete receiver2;

    emitter.emitMySignal("test2");
  }

  // test destructor of Emitter
  {
    MyReceiver receiver;
    receiver.check = &receiver;

    MyEmitter* emitter2 = new MyEmitter;
    Callback::connect(emitter2, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
    delete emitter2;
  }

  // test delete of receiver in slot
  {
    MyEmitter emitter;
    MyReceiver receiver;
    receiver.check = &receiver;
    MyReceiver* receiverSelfDelete = new MyReceiver;
    receiverSelfDelete->check = receiverSelfDelete;
    Callback::connect(&emitter, &MyEmitter::mySignal, receiverSelfDelete, &MyReceiver::selfDelete);
    Callback::connect(&emitter, &MyEmitter::mySignal, &receiver, &MyReceiver::mySlot);
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
    Callback::connect(&emitter, &MyEmitter::mySignal, &receiver1, &MyReceiver::selfDisconnect);
    Callback::connect(&emitter, &MyEmitter::mySignal, &receiver2, &MyReceiver::mySlot);
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
    Callback::connect(emitter, &MyEmitter::mySignal, &receiver1, &MyReceiver::emitterDelete);
    Callback::connect(emitter, &MyEmitter::mySignal, &receiver2, &MyReceiver::mySlot);
    Callback::connect(&emitter2, &MyEmitter::mySignal, &receiver2, &MyReceiver::mySlot);
    emitter->emitMySignal("test2");
    emitter2.emitMySignal("test2");
  }

  // test signal/slot with 8 arguments
  {
    class MyEmitter8 : public Callback::Emitter
    {
    public:
      virtual ~MyEmitter8() {}
      void emitMySignal()
      {
        emit(&MyEmitter8::mySignal, 1, 2, 3, 4, 5, 6, 7, 8);
      }
    public: // signals
      void mySignal(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {}
    };

    class MyReceiver8 : public Callback::Listener
    {
    public:
      bool slotCalled;
      MyReceiver8() : slotCalled(false) {}
    public: // slots
      virtual void mySlot(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {slotCalled = true;}
    };

    MyEmitter8 emitter;
    MyReceiver8 receiver;
    Callback::connect(&emitter, &MyEmitter8::mySignal, &receiver, &MyReceiver8::mySlot);
    emitter.emitMySignal();
    Callback::disconnect(&emitter, &MyEmitter8::mySignal, &receiver, &MyReceiver8::mySlot);
    emitter.emitMySignal();
  }
}

int main(int argc, char* argv[])
{
  testSimpleConnectAndDisconnect();
  testCallback();
  return 0;
}
