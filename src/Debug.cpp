
#ifdef _MSC_VER
#include <Windows.h>
#endif
#include <cstdio>
#include <cstdarg>
#include <malloc.h>

#include <nstd/Debug.h>
#include <nstd/Memory.h>

int_t Debug::print(const tchar_t* str)
{
#ifdef _MSC_VER
  OutputDebugString(str);
#ifdef _UNICODE
  return (int_t)wcslen(str);
#else
  return (int_t)strlen(str);
#endif
#else
  return fputs(str, stderr);
#endif
}

int_t Debug::printf(const tchar_t* format, ...)
{
#ifdef _MSC_VER
  va_list ap;
  va_start(ap, format);
  {
    tchar_t buffer[4096];
#if _UNICODE
    int_t result = _vsnwprintf(buffer, sizeof(buffer), format, ap);
#else
    int_t result = vsnprintf(buffer, sizeof(buffer), format, ap);
#endif
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
#ifdef _UNICODE
    result = _vscwprintf(format, ap);
#else
    result = _vscprintf(format, ap);
#endif
    size_t maxCount = result + 1;
    tchar_t* buffer, *allocatedBuffer = 0;
    try // try using stack buffer
    {
      buffer = (tchar_t*)alloca(maxCount * sizeof(tchar_t));
    }
    catch(...) // fall back on heap buffer
    {
      buffer = (tchar_t*)Memory::alloc(maxCount * sizeof(tchar_t));
      allocatedBuffer = buffer;
    }
#ifdef _UNICODE
    result = _vsnwprintf(buffer, maxCount, format, ap);
#else
    result = vsnprintf(buffer, maxCount, format, ap);
#endif
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
