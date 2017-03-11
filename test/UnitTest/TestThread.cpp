
#include <nstd/Debug.h>
#include <nstd/Thread.h>
#include <nstd/Atomic.h>

void testThread()
{
  // test start and join
  {
    struct ThreadData
    {
      static uint proc(void* args)
      {
        ThreadData& threadData = *(ThreadData*)args;

        ASSERT(threadData.intParam == 123);
        for (int i = 0; i < 10000; ++i)
        {
          Atomic::increment(threadData.counter);
          Thread::yield();
        }
        return 42;
      }
      uint intParam;
      volatile uint counter;
    } threadData;
    threadData.intParam = 123;
    threadData.counter = 0;

    Thread thread;
    Thread thread2;
    Thread thread3;
    ASSERT(thread.start(ThreadData::proc, &threadData));
    ASSERT(thread2.start(ThreadData::proc, &threadData));
    ASSERT(thread3.start(ThreadData::proc, &threadData));
    ASSERT(thread.join() == 42);
    ASSERT(thread.start(ThreadData::proc, &threadData));
    ASSERT(thread.join() == 42);
    ASSERT(thread2.join() == 42);
    ASSERT(thread3.join() == 42);
    ASSERT(threadData.counter == 4 * 10000);
  }

  // test member function start
  {
    struct ThreadData
    {
    public:
      uint proc()
      {
        return 42;
      }
      uint counter;
    } threadData;
    Thread thread;
    threadData.counter = 0;
    ASSERT(thread.start(&threadData, &ThreadData::proc));
    ASSERT(thread.join() == 42);
  }
}
