
#include <nstd/Semaphore.hpp>
#include <nstd/Debug.hpp>
#include <nstd/Time.hpp>

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

int main(int argc, char* argv[])
{
  testSempahore();
  return 0;
}
