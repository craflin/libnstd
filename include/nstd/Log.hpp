
#pragma once

#include <nstd/String.hpp>

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

  enum Device
  {
    stdOutErr, ///< Write log messages up to log level warning to stdout and errors to stderr
    syslog ///< Write log messages to syslog (on Linux)
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
  static void setFormat(const String& lineFormat, const String& timeFormat = String("%H:%M:%S"));

  /**
   * Controls how log messages are handled.
   * The default is @c Log::stdOutErr.
   *
   * @param [in] device The log output device.
   */
  static void setDevice(Device device);

  /**
   * Sets the log level minimum .
   * Log messages smaller than this level are ignored.
   *
   * @param [in] level  The log level.
   */
  static void setLevel(int level);

  static void logf(int level, const char* format, ...);
  static void debugf(const char* format, ...);
  static void infof(const char* format, ...);
  static void warningf(const char* format, ...);
  static void errorf(const char* format, ...);
};
