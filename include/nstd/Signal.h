
#pragma once

#include <nstd/Base.h>

class Signal
{
public:
  Signal(bool set = false);
  ~Signal();

  void_t set();
  void_t reset();
  bool_t wait();
  bool_t wait(timestamp_t timeout);

private:
#ifdef _WIN32
  void* handle;
#else
  #ifdef _AMD64
  byte_t cdata[48]; // sizeof(pthread_cond_t)
  byte_t mdata[40]; // sizeof(pthread_mutex_t)
  #else
  byte_t cdata[48]; // sizeof(pthread_cond_t)
  byte_t mdata[24]; // sizeof(pthread_mutex_t)
  #endif
  bool_t signaled;
#endif

  Signal(const Signal&);
  Signal& operator=(const Signal&);
};
