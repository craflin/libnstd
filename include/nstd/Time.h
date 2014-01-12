

#pragma once

#include <nstd/Base.h>

class Time
{
public:
  /** Retrieve local system time. The function returns the local system time in milliseconds since 1 January 1970 (Unix time with millisesond precision).
  * @return The current local system time (in millisconds).
  */
  static timestamp_t time();

  /** Retrieve ticks (in milliseconds) that have passed since the system was started.
  * @return The ticks (in millisconds) that have currently passed since the system was started.
  */
  static timestamp_t ticks();
};
