
#pragma once

#include <nstd/Base.h>

class Thread
{
public:
  Thread();
  ~Thread();

  bool start(uint (*proc)(void*), void* param);

  uint join();

  static void yield();
  static void sleep(int64 milliseconds);

  /**
  * Get the id of the calling thread.
  *
  * @return The thread id.
  */
  static uint32 getCurrentThreadId();

private:
  void* thread;
  //uint threadId;

  Thread(const Thread&);
  Thread& operator=(const Thread&);
};
