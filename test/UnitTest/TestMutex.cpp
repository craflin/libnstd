
#include <nstd/Debug.hpp>
#include <nstd/Mutex.hpp>

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
