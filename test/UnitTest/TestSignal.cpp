
#include <nstd/Debug.h>
#include <nstd/Signal.h>
#include <nstd/Thread.h>

void_t testSignal()
{
  // test wait with timeout - running into a timeout
  {
    Signal signal;
    ASSERT(!signal.wait(40));
  }

  // test wait with timeout - not running into a timeout
  struct SetSignalProcData
  {
    static uint_t threadProc(void_t* param)
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
    ASSERT(signal.wait(40));
  }

  // test wait without timeout
  {
    Thread thread;
    SetSignalProcData data;
    Signal signal;
    data.testSignal = &signal;
    thread.start(SetSignalProcData::threadProc, &data);

    data.setSignal.set();
    ASSERT(signal.wait(40));
  }
}
