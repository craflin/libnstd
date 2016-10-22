
#pragma once

#include <nstd/String.h>

class Log
{
public:
  enum Level
  {
    debug = 10,
    info = 20,
    warning = 30,
    error = 40,
    critical = 50,
  };

public:
  /**
  * Set logging format.
  * 
  * @param  [in] lineFormat  The log message format. The following placeholders can be used:
  *                          %l - The log level (e.g. "info", "error", "warning", ...)
  *                          %m - The message text.
  *                          %t - The timestamp.
  *                          %P - The process id.
  *                          %T - The id of the calling thread.
  * @param  [in] timeFormat     The format of the timestamp similar to the format string of Time::toString.
  */
  static void setFormat(const String& lineFormat, const String& timeFormat = String(_T("%H:%M:%S")));

  static void setLevel(int level);

  static void logf(int level, const tchar* format, ...);
  static void infof(const tchar* format, ...);
  static void warningf(const tchar* format, ...);
  static void errorf(const tchar* format, ...);
};
