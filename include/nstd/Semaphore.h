/**
* @file
* A classic Semaphore for synchronization in multi thread environments.
* @author Colin Graf
*/

#pragma once

#include <nstd/Base.h>

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
  Semaphore(uint_t value = 0);

  /** Destructor. */
  ~Semaphore();

  /**
  * Increment the semaphore counter.
  */
  void_t signal();

  /**
  * Decrement the semaphore counter. The method returns immediately if the
  * counter is greater than zero. Otherwise it blocks the execution of the
  * calling thread until another thread increases the semaphore counter.
  * @return Whether the semaphore counter was successfully decremented or not.
  */
  bool_t wait();

  /**
  * Decrement the semaphore counter. The method returns immediately if the
  * counter is greater than zero. Otherwise it blocks the execution of the
  * calling thread until another thread increases the semaphore counter or
  * a timeout occurs.
  * @param timeout The maximum time to wait. (in ms).
  * @return Whether the semaphore counter was successfully decremented or not.
  */
  bool_t wait(int64_t timeout);

  /**
  * Try to decrement the semaphore counter. This method returns immediately.
  * @return Whether the semaphore counter was successfully decremented or not.
  */
  bool_t tryWait();

private:
#ifdef _WIN32
  void_t* handle;
#else
  int64_t data[4]; // sizeof(sem_t)
#endif

  Semaphore(const Semaphore&);
  Semaphore& operator=(const Semaphore&);
};
