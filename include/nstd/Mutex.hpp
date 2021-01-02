
#pragma once

#include <nstd/Base.hpp>

class Mutex
{
public:
  class Guard
  {
  public:
    Guard(Mutex &mutex) : _mutex(mutex) { mutex.lock(); }
    ~Guard() { _mutex.unlock(); }

  private:
    Mutex& _mutex;
  };

public:
  Mutex();
  ~Mutex();

  void lock();
  bool tryLock();
  void unlock();

private:
#ifdef _WIN32
#ifdef _AMD64
  byte data[40]; // sizeof(CRITICAL_SECTION)
#else
  byte data[24]; // sizeof(CRITICAL_SECTION)
#endif
#else
#ifdef _AMD64
  int64 data[5]; // sizeof(pthread_mutex_t)
#else
  int64 data[3]; // sizeof(pthread_mutex_t)
#endif
#endif

  Mutex(const Mutex &);
  Mutex &operator=(const Mutex &);
};
