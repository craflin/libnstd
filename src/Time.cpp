
#include <ctime>
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#include <nstd/Time.h>

Time::Time()
{
  time_t now;
  ::time(&now);
  tm* tm = ::gmtime(&now);
  sec = tm->tm_sec;
  min = tm->tm_min;
  hour = tm->tm_hour;
  mday = tm->tm_mday;
  mon = tm->tm_mon;
  year = tm->tm_year;
  wday = tm->tm_wday;
  yday = tm->tm_yday;
  isDst = !!tm->tm_isdst;
}

Time::Time(timestamp_t time)
{
  time_t now = time / 1000;
  tm* tm = ::gmtime(&now);
  sec = tm->tm_sec;
  min = tm->tm_min;
  hour = tm->tm_hour;
  mday = tm->tm_mday;
  mon = tm->tm_mon;
  year = tm->tm_year;
  wday = tm->tm_wday;
  yday = tm->tm_yday;
  isDst = !!tm->tm_isdst;
}

Time::Time(const Time& other) :
  sec(other.sec),
  min(other.min),
  hour(other.hour),
  mday(other.mday),
  mon(other.mon),
  year(other.year),
  wday(other.wday),
  yday(other.yday),
  isDst(other.isDst) {}

timestamp_t Time::toTimestamp()
{
  tm tm;
  tm.tm_sec = sec;
  tm.tm_min = min;
  tm.tm_hour = hour;
  tm.tm_mday = mday;
  tm.tm_mon = mon;
  tm.tm_year = year;
  tm.tm_wday = wday;
  tm.tm_yday = yday;
  tm.tm_isdst = isDst;
#ifdef _MSC_VER
  return _mkgmtime(&tm) * 1000LL;
#else
  return timegm(&tm) * 1000LL;
#endif
}

timestamp_t Time::time()
{
  return ::time(0) * 1000LL;
}

timestamp_t Time::ticks()
{
#ifdef _WIN32
  return GetTickCount();
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (timestamp_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}

String Time::toString(timestamp_t time, const tchar_t* format)
{
  if(!*format)
    return String();
  time_t timet = (time_t)(time / 1000);
  const tm* tms = localtime(&timet);
  String result(256);
  if(!tms)
    return result;
  tchar_t* buffer;
  size_t len;
  for(;;)
  {
    buffer = result;
#ifdef _UNICODE
    len = wcsftime(buffer, result.capacity(), format, tms);
#else
    len = strftime(buffer, result.capacity(), format, tms);
#endif
    if(len > 0)
      break;
    result.reserve(result.capacity() * 2);
  }
  result.resize(len);
  return result;
}
