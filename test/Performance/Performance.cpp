
#include <nstd/Console.h>
#include <nstd/Time.h>

void testMapStd(int iterations);
void_t testMapNStd(int iterations);
void testHashMapStd(int iterations);
void_t testHashMapNStd(int iterations);
void testStringStd(int iterations);
void_t testStringNStd(int iterations);


const int mapTestIterations = 1000000;

int_t main(int_t argc, char_t* argv[])
{
  for (int i = 0; i < 3; ++i)
  {
    {
      Console::printf(_T("testMapStd...  "));
      int64_t startTime = Time::microTicks();
      testMapStd(mapTestIterations);
      int64_t duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }

    {
      Console::printf(_T("testMapNStd... "));
      int64_t startTime = Time::microTicks();
      testMapNStd(mapTestIterations);
      int64_t duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }
    {
      Console::printf(_T("testHashMapStd...  "));
      int64_t startTime = Time::microTicks();
      testHashMapStd(mapTestIterations);
      int64_t duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }

    {
      Console::printf(_T("testHashMapNStd... "));
      int64_t startTime = Time::microTicks();
      testHashMapNStd(mapTestIterations);
      int64_t duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }
    {
      Console::printf(_T("testStringStd...  "));
      int64_t startTime = Time::microTicks();
      testStringStd(mapTestIterations);
      int64_t duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }

    {
      Console::printf(_T("testStringNStd... "));
      int64_t startTime = Time::microTicks();
      testStringNStd(mapTestIterations);
      int64_t duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }
  }

  return 0;
}
