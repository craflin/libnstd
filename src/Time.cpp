
#include <nstd/Time.h>

#include <ctime>

time_t Time::time()
{
  return ::time(0);
}
