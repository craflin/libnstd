

#pragma once

#include <nstd/String.hpp>

class Time
{
public:
  int sec; /**< 0 - 59 */
  int min; /**< 0 - 59 */
  int hour; /**< 0 - 23 */
  int day; /**< 1 - 31 */
  int month; /**< 1 - 12 */
  int year;
  int wday;  /**< The day of week (0 - 6, 0 = Sunday). */
  int yday; /**< The day of year (0 - 365, 0 = First day of the year). */
  bool dst;
  bool utc;

  explicit Time(bool utc = false);
  Time(int64 time, bool utc = false);
  Time(const Time& other);

  Time& toUtc();
  Time& toLocal();

  String toString(const tchar* format);

  int64 toTimestamp();

  bool operator==(const Time& other) const;
  bool operator!=(const Time& other) const;

  /**
  * Retrieve local system time. The function returns the local system time in milliseconds since 1 January 1970 (Unix time with millisesond precision).
  * @return The current local system time (in milliseconds).
  */
  static int64 time();

  /**
  * Retrieve ticks (in milliseconds) that have passed since the system was started.
  * @return The ticks (in milliseconds) that have currently passed since the system was started.
  */
  static int64 ticks();

  /**
  * Retrieve a high resolution time stamp.
  * @return The high resolution time stamp (in microseconds).
  */
  static int64 microTicks();

  static String toString(int64 time, const tchar* format, bool utc = false);

private:
  class Private;
};
