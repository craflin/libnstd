
#pragma once

#include <nstd/Base.h>

class Mutex
{
public:
  Mutex();
  ~Mutex();

  void_t lock();
  bool_t tryLock();
  void_t unlock();

private:
#ifdef _MSC_VER
  #ifdef _M_AMD64
  byte_t data[40];
  #else
  byte_t data[24];
  #endif
#else
#endif
};
