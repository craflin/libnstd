
#include <ctime>
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <nstd/Time.hpp>
#ifdef _WIN32
#include <nstd/Debug.hpp>
#endif

class Time::Private
{
#ifdef _WIN32
public:
  static int64 perfFreq;
  static class Framework
  {
  public:
    Framework()
    {
      LARGE_INTEGER li;
      VERIFY(QueryPerformanceFrequency(&li));
      perfFreq = li.QuadPart / 1000000LL;
    }
  } framework;
#endif
};

#ifdef _WIN32
#ifdef _MSC_VER
#pragma warning(disable: 4073) 
#pragma init_seg(lib)
Time::Private::Framework Time::Private::framework;
#else
Time::Private::Framework Time::Private::framework __attribute__ ((init_priority (101)));
#endif
int64 Time::Private::perfFreq;
#endif

Time::Time(bool utc) : utc(utc)
{
  time_t now;
  ::time(&now);
#ifdef _WIN32
  tm* tm = utc ? ::gmtime(&now) : ::localtime(&now); // win32 gmtime and localtime are thread save
#else
  struct tm tmBuf;
  tm* tm = utc ? ::gmtime_r(&now, &tmBuf) : ::localtime_r(&now, &tmBuf);
#endif
  sec = tm->tm_sec;
  min = tm->tm_min;
  hour = tm->tm_hour;
  day = tm->tm_mday;
  month = tm->tm_mon + 1;
  year = tm->tm_year + 1900;
  wday = tm->tm_wday;
  yday = tm->tm_yday;
  dst = !!tm->tm_isdst;
}

Time::Time(int64 time, bool utc) : utc(utc)
{
  time_t now = (time_t)(time / 1000LL);
#ifdef _WIN32
  tm* tm = utc ? ::gmtime(&now) : ::localtime(&now); // win32 gmtime and localtime are thread save
#else
  struct tm tmBuf;
  tm* tm = utc ? ::gmtime_r(&now, &tmBuf) : ::localtime_r(&now, &tmBuf);
#endif
  sec = tm->tm_sec;
  min = tm->tm_min;
  hour = tm->tm_hour;
  day = tm->tm_mday;
  month = tm->tm_mon + 1;
  year = tm->tm_year + 1900;
  wday = tm->tm_wday;
  yday = tm->tm_yday;
  dst = !!tm->tm_isdst;
}

Time::Time(const Time& other) :
  sec(other.sec),
  min(other.min),
  hour(other.hour),
  day(other.day),
  month(other.month),
  year(other.year),
  wday(other.wday),
  yday(other.yday),
  dst(other.dst),
  utc(other.utc) {}

int64 Time::toTimestamp()
{
  tm tm;
  tm.tm_sec = sec;
  tm.tm_min = min;
  tm.tm_hour = hour;
  tm.tm_mday = day;
  tm.tm_mon = month - 1;
  tm.tm_year = year - 1900;
  tm.tm_wday = wday;
  tm.tm_yday = yday;
  tm.tm_isdst = dst;
  if(utc)
#ifdef _MSC_VER
    return _mkgmtime(&tm) * 1000LL;
#else
    return timegm(&tm) * 1000LL;
#endif
  else
    return mktime(&tm) * 1000LL;
}

Time& Time::toUtc()
{
  if(!utc)
  {
    int64 timestamp = toTimestamp();
    *this = Time(timestamp, true);
  }
  return *this;
}

Time& Time::toLocal()
{
  if(utc)
  {
    int64 timestamp = toTimestamp();
    *this = Time(timestamp, false);
  }
  return *this;
}

bool Time::operator==(const Time& other) const
{
  return sec == other.sec &&
    min == other.min &&
    hour == other.hour &&
    day == other.day &&
    month == other.month &&
    year == other.year &&
    wday == other.wday &&
    yday == other.yday &&
    dst == other.dst &&
    utc == other.utc;
}

bool Time::operator!=(const Time& other) const
{
  return sec != other.sec ||
    min != other.min ||
    hour != other.hour ||
    day != other.day ||
    month != other.month ||
    year != other.year ||
    wday != other.wday ||
    yday != other.yday ||
    dst != other.dst ||
    utc != other.utc;
}

int64 Time::time()
{
  return ::time(0) * 1000LL;
}

int64 Time::ticks()
{
#ifdef _WIN32
  return GetTickCount64();
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (int64)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}

int64 Time::microTicks()
{
#ifdef _WIN32
  LARGE_INTEGER li;
  VERIFY(QueryPerformanceCounter(&li));
  return li.QuadPart / Private::perfFreq;
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (int64)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
#endif
}

String Time::toString(const tchar* format)
{
  if(!*format)
    return String();
  tm tm;
  tm.tm_sec = sec;
  tm.tm_min = min;
  tm.tm_hour = hour;
  tm.tm_mday = day;
  tm.tm_mon = month - 1;
  tm.tm_year = year - 1900;
  tm.tm_wday = wday;
  tm.tm_yday = yday;
  tm.tm_isdst = dst;

  String result(256);
  tchar* buffer;
  usize len;
  for(;;)
  {
    buffer = result;
#ifdef _UNICODE
    len = wcsftime(buffer, result.capacity(), format, &tm);
#else
    len = strftime(buffer, result.capacity(), format, &tm);
#endif
    if(len > 0)
      break;
    result.reserve(result.capacity() * 2);
  }
  result.resize(len);
  return result;
}

String Time::toString(int64 time, const tchar* format, bool utc)
{
  if(!*format)
    return String();
  time_t timet = (time_t)(time / 1000);
  const tm* tms = utc ? ::gmtime(&timet) : ::localtime(&timet);
  String result(256);
  if(!tms)
    return result;
  tchar* buffer;
  usize len;
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
