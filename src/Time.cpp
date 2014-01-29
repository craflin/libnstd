
#include <ctime>
#ifdef _WIN32
#include <Windows.h>
#endif

#include <nstd/Time.h>

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
