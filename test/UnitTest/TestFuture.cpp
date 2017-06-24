
#include <nstd/Debug.h>
#include <nstd/Future.h>
#include <nstd/String.h>

void testFuture()
{
  class FutureTest
  {
  public:
    bool called;

  public:
    FutureTest() : check(42), called(false) {}

    static usize staticTest1(const String& str)
    {
      return str.length();
    }

    usize memberTest1(const String& str)
    {
      return str.length() + check;
    }

    static void staticVoidTest(int check)
    {
      ASSERT(check == 32);
    }

    void memberVoidTest(int check)
    {
      called = true;
      ASSERT(this->check + check == 42 + 32);
    }

  private:
    int check;
  };

  {
    Future<usize> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    future.start(&FutureTest::staticTest1, String("hello"));
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
    ASSERT(future == 5);
  }

  {
    Future<usize> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    FutureTest t;
    future.start(t, &FutureTest::memberTest1, String("hello"));
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
    ASSERT(future == 42 + 5);
  }

  {
    Future<void> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    future.start(&FutureTest::staticVoidTest, 32);
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
  }

  {
    Future<void> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    FutureTest t;
    future.start(t, &FutureTest::memberVoidTest, 32);
    future.join();
    ASSERT(t.called);
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
  }

}
