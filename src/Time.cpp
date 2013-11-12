
#include <ctime>

#include <nstd/Time.h>

timestamp_t Time::time()
{
  return ::time(0) * 1000;
}
