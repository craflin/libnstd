

#pragma once

#include <nstd/String.h>

class Time
{
public:
  int_t sec;
  int_t min;
  int_t hour;
  int_t mday;
  int_t mon;
  int_t year;
  int_t wday;
  int_t yday;
  bool_t isDst;

  Time();
  Time(timestamp_t time);
  Time(const Time& other);

  timestamp_t toTimestamp();

  /** Retrieve local system time. The function returns the local system time in milliseconds since 1 January 1970 (Unix time with millisesond precision).
  * @return The current local system time (in millisconds).
  */
  static timestamp_t time();

  /** Retrieve ticks (in milliseconds) that have passed since the system was started.
  * @return The ticks (in millisconds) that have currently passed since the system was started.
  */
  static timestamp_t ticks();

  static String toString(timestamp_t time, const tchar_t* format);
};
