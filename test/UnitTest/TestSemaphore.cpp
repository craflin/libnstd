
#include <nstd/Debug.h>
#include <nstd/Semaphore.h>
#include <nstd/Time.h>

void_t testSempahore()
{
  Semaphore sem(3);
  sem.signal();
  ASSERT(sem.wait());
  ASSERT(sem.wait());
  ASSERT(sem.wait());
  ASSERT(sem.wait());
  ASSERT(!sem.tryWait());
  int64_t start = Time::ticks();
  ASSERT(!sem.wait(300));
  int64_t waitTime = Time::ticks() - start;
  ASSERT(waitTime > 200);
}
