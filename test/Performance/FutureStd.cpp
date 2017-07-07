
#include <future>
#include <cassert>
#include <cstdio>

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
  if(result != (int64_t)iterations * (int64_t)(iterations - 1) / 2)
  {
    printf("fail\n");
    std::terminate();
  }
}
