
#pragma once

#include <nstd/Base.hpp>

class Signal
{
public:
  Signal(bool set = false);
  ~Signal();

  void set();
  void reset();
  bool wait();
  bool wait(int64 timeout);

private:
#ifdef _WIN32
  void* handle;
#else
  #ifdef _AMD64
  int64 cdata[6]; // sizeof(pthread_cond_t)
  int64 mdata[5]; // sizeof(pthread_mutex_t)
  #else
  int64 cdata[6]; // sizeof(pthread_cond_t)
  int64 mdata[3]; // sizeof(pthread_mutex_t)
  #endif
  bool signaled;
#endif

  Signal(const Signal&);
  Signal& operator=(const Signal&);
};
