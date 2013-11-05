
#include <nstd/Time.h>

#include <ctime>

timestamp_t Time::time()
{
  return ::time(0) * 1000;
}
