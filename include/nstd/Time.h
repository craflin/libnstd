

#pragma once

#include <nstd/String.h>

class Time
{
public:
  int_t sec; /**< 0 – 59 */
  int_t min; /**< 0 – 59 */
  int_t hour; /**< 0 – 23 */
  int_t day; /**< 1 – 31 */
  int_t month; /**< 1 - 12 */
  int_t year;
  int_t wday;  /**< The day of week (0 – 6, 0 = Sunday). */
  int_t yday; /**< The day of year (0 – 365, 0 = First day of the year). */
  bool_t dst;
  bool_t utc;

  explicit Time(bool_t utc = false);
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

  static timestamp_t microTicks();

  static String toString(timestamp_t time, const tchar_t* format, bool_t utc = false);
};
