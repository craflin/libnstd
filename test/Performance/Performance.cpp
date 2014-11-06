
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
      timestamp_t startTime = Time::microTicks();
      testMapStd(mapTestIterations);
      timestamp_t duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }

    {
      Console::printf(_T("testMapNStd... "));
      timestamp_t startTime = Time::microTicks();
      testMapNStd(mapTestIterations);
      timestamp_t duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }
    {
      Console::printf(_T("testHashMapStd...  "));
      timestamp_t startTime = Time::microTicks();
      testHashMapStd(mapTestIterations);
      timestamp_t duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }

    {
      Console::printf(_T("testHashMapNStd... "));
      timestamp_t startTime = Time::microTicks();
      testHashMapNStd(mapTestIterations);
      timestamp_t duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }
    {
      Console::printf(_T("testStringStd...  "));
      timestamp_t startTime = Time::microTicks();
      testStringStd(mapTestIterations);
      timestamp_t duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }

    {
      Console::printf(_T("testStringNStd... "));
      timestamp_t startTime = Time::microTicks();
      testStringNStd(mapTestIterations);
      timestamp_t duration = Time::microTicks() - startTime;
      Console::printf(_T("%lld microseconds\n"), duration);
    }
  }

  return 0;
}
