
#include <nstd/Debug.h>
#include <nstd/Future.h>
#include <nstd/String.h>

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

private:
  int check;
};

void testFuture()
{
  Future<usize> future;
  ASSERT(!future.isAborting());
  ASSERT(!future.isFinished());
  ASSERT(!future.isAborted());
  future.start(&FutureTest::test2, String("hello"));
  future.join();
  ASSERT(!future.isAborting());
  ASSERT(future.isFinished());
  ASSERT(!future.isAborted());
  ASSERT(future == 5);
}
