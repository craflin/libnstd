
#if defined(_WIN32)
#include <Windows.h>
#ifndef NDEBUG
#include <Dbghelp.h>
#endif
#else
#ifndef NDEBUG
#include <dlfcn.h>
#endif
#endif
#include <cstdio>
#include <cstdarg>

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
    tchar_t* buffer = (tchar_t*)Memory::alloc(maxCount * sizeof(tchar_t));
#ifdef _UNICODE
    result = _vsnwprintf(buffer, maxCount, format, ap);
#else
    result = vsnprintf(buffer, maxCount, format, ap);
#endif
    va_end(ap);
    OutputDebugString(buffer);
    Memory::free(buffer);
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

#ifndef NDEBUG
bool_t Debug::getSourceLine(void* addr, const tchar_t*& file, int_t& line)
{
#ifdef _WIN32
  typedef BOOL (WINAPI *PSymInitialize)(HANDLE hProcess, PCSTR UserSearchPath, BOOL fInvadeProcess);
  typedef BOOL (WINAPI *PSymGetLineFromAddr64)(HANDLE hProcess, DWORD64 qwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line64);
  static PSymInitialize pSymInitialize = 0;
  static PSymGetLineFromAddr64 pSymGetLineFromAddr64 = 0;
  static bool initialized = false;
  HANDLE hProcess = GetCurrentProcess();
  if(!initialized)
  {
    initialized = true;
    HMODULE hModule = LoadLibrary("Dbghelp.dll");
    if(!hModule)
      return false;
    pSymInitialize = (PSymInitialize)GetProcAddress(hModule, "SymInitialize");
    pSymGetLineFromAddr64 = (PSymGetLineFromAddr64)GetProcAddress(hModule, "SymGetLineFromAddr64");
    if(!pSymInitialize || !pSymGetLineFromAddr64)
      return false;
    if(!pSymInitialize(hProcess, NULL, TRUE))
    {
      pSymGetLineFromAddr64 = 0;
      return false;
    }
  }
  if(!pSymGetLineFromAddr64)
    return false;
  IMAGEHLP_LINE64 ihLine;
  ihLine.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
  DWORD displacement;
  if(!pSymGetLineFromAddr64(hProcess, (DWORD64)addr, &displacement, &ihLine))
    return false;
#ifdef UNICODE
  static wchar_t fileName[MAX_PATH];
  mbstowcs(fileName, ihLine.FileName, strlen(ihLine.FileName));
  file = fileName;
#else
  file = ihLine.FileName;
#endif
  line = ihLine.LineNumber;
  return true;
#else
  return false;
  //Dl_info dli;
  //if(dladdr(addr, &dli) == 0)
  //  return false;
  //file = dli.dli_sname;
  //line = 0;
  //return true;
#endif
}
#endif
