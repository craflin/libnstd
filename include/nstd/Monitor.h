
#pragma once

#include <nstd/Base.h>

class Monitor
{
public:
  Monitor();
  ~Monitor();

  bool_t tryLock();
  void_t lock();
  bool_t wait();
  bool_t wait(int64_t timeout);
  void_t unlock();
  void_t set();

private:
  bool_t signaled;
#ifdef _WIN32
  #ifdef _AMD64
  byte_t cdata[8]; // sizeof(CONDITION_VARIABLE)
  byte_t mdata[40]; // sizeof(CRITICAL_SECTION)
  #else
  byte_t cdata[4]; // sizeof(CONDITION_VARIABLE)
  byte_t mdata[24]; // sizeof(CRITICAL_SECTION)
  #endif
#else
  #ifdef _AMD64
  byte_t cdata[48]; // sizeof(pthread_cond_t)
  byte_t mdata[40]; // sizeof(pthread_mutex_t)
  #else
  byte_t cdata[48]; // sizeof(pthread_cond_t)
  byte_t mdata[24]; // sizeof(pthread_mutex_t)
  #endif
#endif

  Monitor(const Monitor&);
  Monitor& operator=(const Monitor&);
};


