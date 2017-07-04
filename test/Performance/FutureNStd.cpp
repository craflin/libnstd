
#include <nstd/Future.h>

void testFutureNStd(int iterations)
{
  struct A
  {
    static int64 testProc(int64 i)
    {
      return i;
    }
  };

  int64 result = 0;
  Future<int64> future;
  for(int i = 0; i < iterations; ++i)
  {
    future.start(&A::testProc, i);
    result += future;
  }
}
