
#pragma once

#include <nstd/Base.hpp>

class Monitor
{
public:
  class Guard
  {
  public:
    Guard(Monitor &monitor) : _monitor(monitor) { monitor.lock(); }
    ~Guard() { _monitor.unlock(); }

    bool wait() {return _monitor.wait();}
    bool wait(int64 timeout) {return _monitor.wait(timeout);}

  private:
    Monitor& _monitor;
  };

public:
  Monitor();
  ~Monitor();

  bool tryLock();
  void lock();
  bool wait();
  bool wait(int64 timeout);
  void unlock();
  void set();

private:
  bool signaled;
#ifdef _WIN32
  #ifdef _AMD64
  byte cdata[8]; // sizeof(CONDITION_VARIABLE)
  byte mdata[40]; // sizeof(CRITICAL_SECTION)
  #else
  byte cdata[4]; // sizeof(CONDITION_VARIABLE)
  byte mdata[24]; // sizeof(CRITICAL_SECTION)
  #endif
#else
  #ifdef _AMD64
  int64 cdata[6]; // sizeof(pthread_cond_t)
  int64 mdata[5]; // sizeof(pthread_mutex_t)
  #else
  int64 cdata[6]; // sizeof(pthread_cond_t)
  int64 mdata[3]; // sizeof(pthread_mutex_t)
  #endif
#endif

  Monitor(const Monitor&);
  Monitor& operator=(const Monitor&);
};


