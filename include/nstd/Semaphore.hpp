/**
* @file
* A classic Semaphore for synchronization in multi thread environments.
* @author Colin Graf
*/

#pragma once

#include <nstd/Base.hpp>

/**
* Encapsulation of a semaphore object.
*/
class Semaphore
{
public:
  /**
  * Construct a new semaphore object.
  * @param value The initial value of the semaphore counter.
  */
  Semaphore(uint value = 0);

  /** Destructor. */
  ~Semaphore();

  /**
  * Increment the semaphore counter.
  */
  void signal();

  /**
  * Decrement the semaphore counter. The method returns immediately if the
  * counter is greater than zero. Otherwise it blocks the execution of the
  * calling thread until another thread increases the semaphore counter.
  * @return Whether the semaphore counter was successfully decremented or not.
  */
  bool wait();

  /**
  * Decrement the semaphore counter. The method returns immediately if the
  * counter is greater than zero. Otherwise it blocks the execution of the
  * calling thread until another thread increases the semaphore counter or
  * a timeout occurs.
  * @param timeout The maximum time to wait. (in ms).
  * @return Whether the semaphore counter was successfully decremented or not.
  */
  bool wait(int64 timeout);

  /**
  * Try to decrement the semaphore counter. This method returns immediately.
  * @return Whether the semaphore counter was successfully decremented or not.
  */
  bool tryWait();

private:
#ifdef _WIN32
  void* handle;
#else
  int64 data[4]; // sizeof(sem_t)
#endif

  Semaphore(const Semaphore&);
  Semaphore& operator=(const Semaphore&);
};
