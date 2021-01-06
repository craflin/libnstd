
#include <nstd/Signal.hpp>
#include <nstd/Debug.hpp>
#include <nstd/Thread.hpp>

void testSignal()
{
  // test wait with timeout - running into a timeout
  {
    Signal signal;
    ASSERT(!signal.wait(20));
  }

  // test wait with timeout - not running into a timeout
  struct SetSignalProcData
  {
    static uint threadProc(void* param)
    {
      SetSignalProcData* data = (SetSignalProcData*)param;
      data->setSignal.wait();
      data->testSignal->set();
      return 0;
    }
    Signal setSignal;
    Signal* testSignal;
  };

  {
    Thread thread;
    SetSignalProcData data;
    Signal signal;
    data.testSignal = &signal;
    thread.start(SetSignalProcData::threadProc, &data);
    data.setSignal.set();
    ASSERT(signal.wait(2000));
  }

  // test wait without timeout
  {
    Thread thread;
    SetSignalProcData data;
    Signal signal;
    data.testSignal = &signal;
    thread.start(SetSignalProcData::threadProc, &data);
    data.setSignal.set();
    ASSERT(signal.wait());
  }
}

int main(int argc, char* argv[])
{
  testSignal();
  return 0;
}
