

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
  bool_t dst;
  bool_t utc;

  Time(bool_t utc = false);
  Time(timestamp_t time, bool_t utc = false);
  Time(const Time& other);

  Time& toUtc();
  Time& toLocal();

  String toString(const tchar_t* format);

  timestamp_t toTimestamp();

  bool_t operator==(const Time& other) const;
  bool_t operator!=(const Time& other) const;

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
