
#include <nstd/Debug.h>
#include <nstd/Future.h>
#include <nstd/String.h>

void testFuture()
{
  class FutureTest
  {
  public:
    FutureTest() : check(42) {}

    usize test1(const String& str)
    {
      return str.length() + check;
    }

    static usize test2(const String& str)
    {
      return str.length();
    }

    static void testVoid(int check)
    {
      ASSERT(check == 32);
    }

  private:
    int check;
  };

  /*
  {
    Future<usize> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    future.start2(&FutureTest::test2, String("hello"));
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
    ASSERT(future == 5);
  }
  */

  {
    Future<void> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    future.start3(&FutureTest::testVoid, 32);
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
  }
}
