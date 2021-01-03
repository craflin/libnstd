
#include <nstd/Mutex.hpp>
#include <nstd/Debug.hpp>

void testMutex()
{
  // test recursion
  {
    Mutex mutex;
    Mutex::Guard guard1(mutex);
    Mutex::Guard guard2(mutex);
  }
}

int main(int argc, char* argv[])
{
  testMutex();
  return 0;
}
