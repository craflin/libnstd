
#include <future>

void testFutureStd(int iterations)
{
  struct A
  {
    static int64_t testProc(int64_t i)
    {
      return i;
    }
  };

  int64_t result = 0;
  for(int i = 0; i < iterations; ++i)
  {
    std::future<int64_t> future = std::async(&A::testProc, i);
    result += future.get();
  }
}
