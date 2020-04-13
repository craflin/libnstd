
#include <nstd/Console.hpp>
#include <nstd/Time.hpp>

void testMapStd(int iterations);
void testMapNStd(int iterations);
void testHashMapStd(int iterations);
void testHashMapNStd(int iterations);
void testStringStd(int iterations);
void testStringNStd(int iterations);
void testFutureStd(int iterations);
void testFutureNStd(int iterations);

const int mapTestIterations = 1000000;

int main(int argc, char* argv[])
{
  for (int i = 0; i < 3; ++i)
  {
    {
      Console::printf(_T("testMapStd...  "));
      int64 startTime = Time::microTicks();
      testMapStd(mapTestIterations);
      int64 duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }

    {
      Console::printf(_T("testMapNStd... "));
      int64 startTime = Time::microTicks();
      testMapNStd(mapTestIterations);
      int64 duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }
    {
      Console::printf(_T("testHashMapStd...  "));
      int64 startTime = Time::microTicks();
      testHashMapStd(mapTestIterations);
      int64 duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }

    {
      Console::printf(_T("testHashMapNStd... "));
      int64 startTime = Time::microTicks();
      testHashMapNStd(mapTestIterations);
      int64 duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }
    {
      Console::printf(_T("testStringStd...  "));
      int64 startTime = Time::microTicks();
      testStringStd(mapTestIterations);
      int64 duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }

    {
      Console::printf(_T("testStringNStd... "));
      int64 startTime = Time::microTicks();
      testStringNStd(mapTestIterations);
      int64 duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }

    {
      Console::printf(_T("testFutureStd...  "));
      int64 startTime = Time::microTicks();
      testFutureStd(mapTestIterations / 10);
      int64 duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }

    {
      Console::printf(_T("testFutureNStd... "));
      int64 startTime = Time::microTicks();
      testFutureNStd(mapTestIterations / 10);
      int64 duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }
  }

  return 0;
}
