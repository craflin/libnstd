
#include <nstd/Debug.h>
#include <nstd/Signal.h>
#include <nstd/Thread.h>
#include <nstd/Monitor.h>

#include <nstd/Console.h>
#include <Windows.h>

void_t testMonitor()
{

  // test wait with timeout - running into a timeout
  {
    Monitor monitor;
    monitor.lock();
    ASSERT(!monitor.wait(40));
    monitor.unlock();
  }

  // test wait with timeout - not running into a timeout
  struct SetMonitorProcData
  {
    static uint_t threadProc(void_t* param)
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

    monitor.lock();
    data.setSignal.set();
    ASSERT(monitor.wait(40));
    monitor.unlock();
  }

  // test wait without timeout
  {
    Thread thread;
    SetMonitorProcData data;
    Monitor monitor;
    data.monitor = &monitor;
    thread.start(SetMonitorProcData::threadProc, &data);

    monitor.lock();
    data.setSignal.set();
    ASSERT(monitor.wait(40));
    monitor.unlock();
  }
}
