
#include <nstd/Console.h>

#include <cstdarg>
#include <cstdio>

int_t Console::print(const char* str)
{
  return fputs(str, stdout);
}

int_t Console::printf(const char_t* format, ...)
{
  va_list ap;
  va_start(ap, format);
  int_t result = vprintf(format, ap);
  va_end(ap);
  return result;
}
