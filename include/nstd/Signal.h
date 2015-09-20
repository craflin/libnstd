
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
  bool_t wait(int64_t timeout);

private:
#ifdef _WIN32
  void* handle;
#else
  #ifdef _AMD64
  int64_t cdata[6]; // sizeof(pthread_cond_t)
  int64_t mdata[5]; // sizeof(pthread_mutex_t)
  #else
  int64_t cdata[6]; // sizeof(pthread_cond_t)
  int64_t mdata[3]; // sizeof(pthread_mutex_t)
  #endif
  bool_t signaled;
#endif

  Signal(const Signal&);
  Signal& operator=(const Signal&);
};
