
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
      Console::printf("testMapStd...  ");
      int64 startTime = Time::microTicks();
      testMapStd(mapTestIterations);
      int64 duration = Time::microTicks() - startTime;
      Console::printf("%lld microseconds\n", duration);
    }

    {
      Console::printf("testMapNStd... ");
      int64 startTime = Time::microTicks();
      testMapNStd(mapTestIterations);
      int64 duration = Time::microTicks() - startTime;
      Console::printf("%lld microseconds\n", duration);
    }
    {
      Console::printf("testHashMapStd...  ");
      int64 startTime = Time::microTicks();
      testHashMapStd(mapTestIterations);
      int64 duration = Time::microTicks() - startTime;
      Console::printf("%lld microseconds\n", duration);
    }

    {
      Console::printf("testHashMapNStd... ");
      int64 startTime = Time::microTicks();
      testHashMapNStd(mapTestIterations);
      int64 duration = Time::microTicks() - startTime;
      Console::printf("%lld microseconds\n", duration);
    }
    {
      Console::printf("testStringStd...  ");
      int64 startTime = Time::microTicks();
      testStringStd(mapTestIterations);
      int64 duration = Time::microTicks() - startTime;
      Console::printf("%lld microseconds\n", duration);
    }

    {
      Console::printf("testStringNStd... ");
      int64 startTime = Time::microTicks();
      testStringNStd(mapTestIterations);
      int64 duration = Time::microTicks() - startTime;
      Console::printf("%lld microseconds\n", duration);
    }

    {
      Console::printf("testFutureStd...  ");
      int64 startTime = Time::microTicks();
      testFutureStd(mapTestIterations / 10);
      int64 duration = Time::microTicks() - startTime;
      Console::printf("%lld microseconds\n", duration);
    }

    {
      Console::printf("testFutureNStd... ");
      int64 startTime = Time::microTicks();
      testFutureNStd(mapTestIterations / 10);
      int64 duration = Time::microTicks() - startTime;
      Console::printf("%lld microseconds\n", duration);
    }
  }

  return 0;
}
