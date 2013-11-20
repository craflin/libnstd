/**
* A Semaphore for thread synchronization.
* @author Colin Graf
*/

#pragma once

#include <nstd/Base.h>

/**
* Encapsulates an semaphore object.
*/
class Semaphore
{
public:
  /**
  * Construct a new semaphore object.
  * @param value The initial value of the semaphore.
  */
  Semaphore(unsigned int value = 0);

  /** Destructor. */
  ~Semaphore();

  /**
  * Increment the semaphore counter.
  */
  void post();

  /**
  * Decrement the semaphore counter. The method returns immediatly if the
  * counter is greater than zero. Otherwise it blocks until the semaphore
  * counter can be decremented.
  * @return Whether the decrementation was successful.
  */
  bool wait();

  /**
  * Decrement the semaphore counter. The method returns immediatly if the
  * counter is greater than zero. Otherwise it blocks until the semaphore
  * counter can be decremented.
  * @param timeout A timeout for the blocking call. (in ms).
  * @return Whether the decrementation was successful.
  */
  bool wait(timestamp_t timeout);

  /**
  * Try to decrement the semaphore counter. This method returns immediatly.
  * @return Whether the decrementation was successful.
  */
  bool tryWait();

private:
  void* handle;
};
