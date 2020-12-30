
#include <nstd/Mutex.hpp>
#include <nstd/Debug.hpp>

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

int main(int argc, char* argv[])
{
  testMutex();
  return 0;
}
