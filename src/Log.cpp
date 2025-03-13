
#include <nstd/Log.hpp>

#ifndef _WIN32
#include <syslog.h>
#endif

#include <cstdarg>
#include <cstdio>

#include <nstd/Mutex.hpp>
#include <nstd/Time.hpp>
#include <nstd/Process.hpp>
#include <nstd/Thread.hpp>
#include <nstd/Console.hpp>
#include <nstd/Debug.hpp>

static class _Log
{
public:
  static Mutex mutex;
  static String lineFormat;
  static String timeFormat;
  static Log::Device device;
  static int level;

#ifndef _WIN32
  ~_Log()
  {
    if(device == Log::syslog)
      ::closelog();
  }

  static int mapLevelToSyslog(int level)
  {
    if(level <= Log::debug)
      return LOG_DEBUG;
    if(level <= Log::info)
      return LOG_INFO;
    if(level <= Log::warning)
      return LOG_WARNING;
    if(level <= Log::error)
      return LOG_ERR;
    return LOG_CRIT;
  }
#endif

  static void vlogf(int level, const char* format, va_list& vl)
  {
    int64 time = Time::time();
    String lineFormat;
    String timeFormat;
    Log::Device device;

    {
      Mutex::Guard guard(_Log::mutex);
      device = _Log::device;
      if (device != Log::syslog)
      {
        lineFormat = _Log::lineFormat;
        timeFormat = _Log::timeFormat;
      }
    }

    // get message
    String data(200);
    int result;
    {
      usize capacity = data.capacity();
      va_list tmp;
      va_copy(tmp, vl);
      result = vsnprintf((char*)data, capacity, format, tmp);
      va_end(tmp);
      if(result >= 0 && result < (int)capacity)
        data.resize(result);
      else // buffer was too small: compute size, reserve buffer, print again
      {
        va_copy(tmp, vl);
#ifdef _MSC_VER
        result = _vscprintf(format, vl);
#else
        result = vsnprintf(0, 0, format, vl);
#endif
        va_end(tmp);
        ASSERT(result >= 0);
        if(result >= 0)
        {
          data.reserve(result);
          va_copy(tmp, vl);
          result = vsnprintf((char*)data, result + 1, format, vl);
          va_end(tmp);
          ASSERT(result >= 0);
          if(result >= 0)
            data.resize(result);
        }
      }
    }

#ifndef _WIN32
    if(_Log::device == Log::syslog)
    {
      syslog(mapLevelToSyslog(level), "%s", (const char*)data);
      return;
    }
#endif

    // build line
    String line(data.length() + 200);
    for(const char* p = lineFormat; *p; ++p)
    {
      if(*p == '%')
      {
        ++p;
        switch(*p)
        {
        case '%':
          line += '%';
          break;
        case 'm':
          line += data;
          break;
        case 't':
          line += Time::toString(time, timeFormat);
          break;
        case 'L':
          switch(level)
          {
          case Log::debug: line += "debug"; break;
          case Log::info: line += "info"; break;
          case Log::warning: line += "warning"; break;
          case Log::error: line += "error"; break;
          case Log::critical: line += "critical"; break;
          default: line += "unknown"; break;
          }
          break;
        case 'P':
          line += String::fromInt(Process::getCurrentProcessId());
          break;
        case 'T':
          line += String::fromInt(Thread::getCurrentThreadId());
          break;
        default:
          line += '%';
          line += *p;
          break;
        }
      }
      else
        line += *p;
    }
    line += '\n';
    if(level >= Log::warning)
      Console::error(line);
    else
      Console::print(line);
  }

} _log;

Mutex _Log::mutex;
String _Log::lineFormat("[%t] %L: %m");
String _Log::timeFormat("%H:%M:%S");
int _Log::level = Log::info;
Log::Device _Log::device = Log::stdOutErr;

void Log::setFormat(const String& lineFormat, const String& timeFormat)
{
  Mutex::Guard guard(_Log::mutex);
  _Log::lineFormat = lineFormat;
  _Log::timeFormat = timeFormat;
}

void Log::setDevice(Device device)
{
  Mutex::Guard guard(_Log::mutex);
  if(_Log::device != device)
  {
#ifndef _WIN32
    if(_Log::device == Log::syslog)
      ::closelog();
#endif
    _Log::device = device;
#ifndef _WIN32
    if(device == Log::syslog)
      ::openlog(NULL, LOG_CONS|LOG_NDELAY|LOG_PID, LOG_DAEMON);
#endif
  }
}

void Log::setLevel(int level)
{
  _Log::level = level;
}

void Log::logf(int level, const char* format, ...)
{
  if(level < _Log::level)
    return;
  va_list vl;
  va_start(vl, format);
  _Log::vlogf(level, format, vl);
  va_end(vl);
}

void Log::debugf(const char* format, ...)
{
  if(Log::debug < _Log::level)
    return;
  va_list vl;
  va_start(vl, format);
  _Log::vlogf(Log::debug, format, vl);
  va_end(vl);
}

void Log::infof(const char* format, ...)
{
  if(Log::info < _Log::level)
    return;
  va_list vl;
  va_start(vl, format);
  _Log::vlogf(Log::info, format, vl);
  va_end(vl);
}

void Log::warningf(const char* format, ...)
{
  if(Log::warning < _Log::level)
    return;
  va_list vl;
  va_start(vl, format);
  _Log::vlogf(Log::warning, format, vl);
  va_end(vl);
}

void Log::errorf(const char* format, ...)
{
  if(Log::error < _Log::level)
    return;
  va_list vl;
  va_start(vl, format);
  _Log::vlogf(Log::error, format, vl);
  va_end(vl);
}
