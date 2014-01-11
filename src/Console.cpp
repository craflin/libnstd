
#include <cstdarg>
#include <cstdio>
#ifdef _MSC_VER
#include <tchar.h>
#endif

#include <nstd/Console.h>

int_t Console::print(const tchar_t* str)
{
#ifdef _MSC_VER
  return _fputts(str, stdout);
#else
  return fputs(str, stdout);
#endif
}

int_t Console::printf(const tchar_t* format, ...)
{
  va_list ap;
  va_start(ap, format);
#ifdef _UNICODE
  int_t result = vwprintf(format, ap);
#else
  int_t result = vprintf(format, ap);
#endif
  va_end(ap);
  return result;
}

int_t Console::error(const tchar_t* str)
{
#ifdef _MSC_VER
  return _fputts(str, stderr);
#else
  return fputs(str, stderr);
#endif
}

int_t Console::errorf(const tchar_t* format, ...)
{
  va_list ap;
  va_start(ap, format);
#ifdef _UNICODE
  int_t result = vfwprintf(stderr, format, ap);
#else
  int_t result = vfprintf(stderr, format, ap);
#endif
  va_end(ap);
  return result;
}
