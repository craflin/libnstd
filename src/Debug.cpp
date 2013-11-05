
#include <nstd/Debug.h>
#include <nstd/Memory.h>

#ifdef _MSC_VER
#include <Windows.h>
#endif
#include <cstdio>
#include <cstdarg>
#include <malloc.h>

int_t Debug::print(const char_t* str)
{
#ifdef _MSC_VER
  OutputDebugString(str);
  return (int_t)strlen(str);
#else
  return fputs(str, stderr);
#endif
}

int_t Debug::printf(const char_t* format, ...)
{
#ifdef _MSC_VER
  va_list ap;
  va_start(ap, format);
  {
    char_t buffer[4096];
    int_t result = vsnprintf(buffer, sizeof(buffer), format, ap);
    if(result >= 0 && result < (int_t)sizeof(buffer))
    {
      OutputDebugString(buffer);
      va_end(ap);
      return result;
    }
  }

  // buffer was too small
  {
    int_t result;
//#ifdef _MSC_VER
    result = _vscprintf(format, ap);
//#else
//    result = vsnprintf(0, 0, format, ap);
//#endif
    size_t bufferSize = result + 1;
    char_t* buffer, *allocatedBuffer = 0;
    try // try using stack buffer
    {
      buffer = (char_t*)alloca(bufferSize);
    }
    catch(...) // fall back on heap buffer
    {
      buffer = (char_t*)Memory::alloc(bufferSize);
      allocatedBuffer = buffer;
    }
    result = vsnprintf(buffer, bufferSize, format, ap);
    va_end(ap);
    OutputDebugString(buffer);
    if(allocatedBuffer)
      Memory::free(allocatedBuffer);
    return result;
  }
#else
  va_list ap;
  va_start(ap, format);
  int_t result = vfprintf(stderr, format, ap);
  va_end(ap);
  return result;
#endif
}
