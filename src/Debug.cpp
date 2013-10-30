
#include <nstd/Debug.h>
#include <nstd/Memory.h>

#include <Windows.h>
#include <cstdio>
#include <malloc.h>

int_t Debug::print(const char* str)
{
  OutputDebugString(str);
  return (int_t)strlen(str);
}

int_t Debug::printf(const char_t* format, ...)
{
  va_list ap;
  va_start(ap, format);
  {
    char_t buffer[4096];
    int_t result = vsnprintf(buffer, sizeof(buffer), format, ap);
    if(result >= 0 && result < sizeof(buffer))
    {
      OutputDebugString(buffer);
      va_end(ap);
      return result;
    }
  }

  // buffer was too small
  {
    int_t result = _vscprintf(format, ap);
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
}
