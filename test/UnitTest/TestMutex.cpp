
#include <nstd/Debug.h>
#include <nstd/Mutex.h>

void testMutex()
{
  // test recursion
  {
    Mutex mutex;
    mutex.lock();
    mutex.lock();
    mutex.unlock();
    mutex.unlock();
  }
}
