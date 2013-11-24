
#include <ctime>
#ifdef _WIN32
#include <Windows.h>
#endif

#include <nstd/Time.h>

timestamp_t Time::time()
{
  return ::time(0) * 1000;
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
