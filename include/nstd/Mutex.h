
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
#ifdef _WIN32
  #ifdef _AMD64
  byte_t data[40]; // sizeof(CRITICAL_SECTION)
  #else
  byte_t data[24]; // sizeof(CRITICAL_SECTION)
  #endif
#else
  #ifdef _AMD64
  byte_t data[40]; // sizeof(pthread_mutex_t)
  #else
  byte_t data[24]; // sizeof(pthread_mutex_t)
  #endif
#endif

  Mutex(const Mutex&);
  Mutex& operator=(const Mutex&);
};
