
#include <nstd/Time.hpp>
#include <nstd/String.hpp>
#include <nstd/Debug.hpp>

void testTime()
{
  String test = Time::toString(123 * 1000, _T("%Y-%m-%d %H:%M:%S"), true);
  ASSERT(test == _T("1970-01-01 00:02:03"));

  {
    Time time(123LL * 1000, true);
    ASSERT(time.toString(_T("%Y-%m-%d %H:%M:%S")) == test);
    ASSERT(time.year == 1970);
    ASSERT(time.month == 1);
    ASSERT(time.day == 1);
    ASSERT(time.hour == 0);
    ASSERT(time.min == 2);
    ASSERT(time.sec == 3);
  }

  int64 now = Time::time();
  Time time(now);
  ASSERT(time.toTimestamp() == now);
  Time time2(time);
  time2.toLocal();
  ASSERT(time == time2);
  ASSERT(time2.toTimestamp() == now);

  Time timeUtc(time);
  timeUtc.toUtc();
  ASSERT(timeUtc != time);
  ASSERT(timeUtc.toTimestamp() == now);
  Time timeUtc2(timeUtc);
  timeUtc2.toUtc();
  ASSERT(timeUtc != time);
  ASSERT(timeUtc2 == timeUtc);
  ASSERT(timeUtc2.toTimestamp() == now);
}

int main(int argc, char* argv[])
{
  testTime();
  return 0;
}
