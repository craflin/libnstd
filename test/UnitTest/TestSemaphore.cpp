
#include <nstd/Debug.h>
#include <nstd/Semaphore.h>
#include <nstd/Time.h>

void testSempahore()
{
  Semaphore sem(3);
  sem.signal();
  ASSERT(sem.wait());
  ASSERT(sem.wait());
  ASSERT(sem.wait());
  ASSERT(sem.wait());
  ASSERT(!sem.tryWait());
  int64 start = Time::ticks();
  ASSERT(!sem.wait(300));
  int64 waitTime = Time::ticks() - start;
  ASSERT(waitTime > 200);
}
