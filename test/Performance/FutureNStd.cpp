
#include <nstd/Future.hpp>
#include <nstd/Console.hpp>
#include <nstd/Process.hpp>

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
  for(int i = 0; i < iterations; ++i)
  {
    Future<int64> future;
    future.start(&A::testProc, i);
    result += future;
  }
  if(result != (int64)iterations * (int64)(iterations - 1) / 2)
  {
    Console::printf(_T("fail\n"));
    Process::exit(1);
  }
}
