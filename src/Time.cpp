
#include <ctime>
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

#include <nstd/Time.h>
#ifdef _WIN32
#include <nstd/Debug.h>
#endif

#ifdef _WIN32
#pragma warning(disable: 4073)
#pragma init_seg(lib)
class _Time
{
public:
  static timestamp_t perfFreq;

private:
  static _Time data;

  _Time()
  {
    LARGE_INTEGER li;
    VERIFY(QueryPerformanceFrequency(&li));
    perfFreq = li.QuadPart / 1000000LL;
  }
};
_Time _Time::data;
timestamp_t _Time::perfFreq;
#endif

Time::Time(bool_t utc) : utc(utc)
{
  time_t now;
  ::time(&now);
  tm* tm = utc ? ::gmtime(&now) : ::localtime(&now);
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

Time::Time(timestamp_t time, bool_t utc) : utc(utc)
{
  time_t now = (time_t)(time / 1000LL);
  tm* tm = utc ? ::gmtime(&now) : ::localtime(&now);
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

timestamp_t Time::toTimestamp()
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
    timestamp_t timestamp = toTimestamp();
    *this = Time(timestamp, true);
  }
  return *this;
}

Time& Time::toLocal()
{
  if(utc)
  {
    timestamp_t timestamp = toTimestamp();
    *this = Time(timestamp, false);
  }
  return *this;
}

bool_t Time::operator==(const Time& other) const
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

bool_t Time::operator!=(const Time& other) const
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

timestamp_t Time::microTicks()
{
#ifdef _WIN32
  LARGE_INTEGER li;
  VERIFY(QueryPerformanceCounter(&li));
  return li.QuadPart / _Time::perfFreq;
#else
  // todo
#endif
}

String Time::toString(const tchar_t* format)
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
  tchar_t* buffer;
  size_t len;
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

String Time::toString(timestamp_t time, const tchar_t* format, bool_t utc)
{
  if(!*format)
    return String();
  time_t timet = (time_t)(time / 1000);
  const tm* tms = utc ? ::gmtime(&timet) : ::localtime(&timet);
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
