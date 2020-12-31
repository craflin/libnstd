
#include <nstd/Monitor.hpp>
#include <nstd/Debug.hpp>
#include <nstd/Signal.hpp>
#include <nstd/Thread.hpp>

void testMonitor()
{
  // test wait with timeout - running into a timeout
  {
    Monitor monitor;
    {
      Monitor::Guard guard(monitor);
      ASSERT(!guard.wait(40));
    }
  }

  // test wait with timeout - not running into a timeout
  struct SetMonitorProcData
  {
    static uint threadProc(void* param)
    {
      SetMonitorProcData* data = (SetMonitorProcData*)param;
      data->setSignal.wait();
      data->monitor->set();
      return 0;
    }
    Signal setSignal;
    Monitor* monitor;
  };

  {
    Thread thread;
    SetMonitorProcData data;
    Monitor monitor;
    data.monitor = &monitor;
    thread.start(SetMonitorProcData::threadProc, &data);

    {
      Monitor::Guard guard(monitor);
      data.setSignal.set();
      ASSERT(guard.wait(40));
    }
  }

  // test wait without timeout
  {
    Thread thread;
    SetMonitorProcData data;
    Monitor monitor;
    data.monitor = &monitor;
    thread.start(SetMonitorProcData::threadProc, &data);

    {
      Monitor::Guard guard(monitor);
      data.setSignal.set();
      ASSERT(guard.wait(40));
    }
  }
}

int main(int argc, char* argv[])
{
  testMonitor();
  return 0;
}
