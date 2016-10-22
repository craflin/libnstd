
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

int Debug::print(const tchar* str)
{
#ifdef _MSC_VER
  OutputDebugString(str);
#ifdef _UNICODE
  return (int)wcslen(str);
#else
  return (int)strlen(str);
#endif
#else
  return fputs(str, stderr);
#endif
}

int Debug::printf(const tchar* format, ...)
{
#ifdef _MSC_VER
  va_list ap;
  va_start(ap, format);
  {
    tchar buffer[4096];
#if _UNICODE
    int result = _vsnwprintf(buffer, sizeof(buffer), format, ap);
#else
    int result = vsnprintf(buffer, sizeof(buffer), format, ap);
#endif
    if(result >= 0 && result < (int)sizeof(buffer))
    {
      OutputDebugString(buffer);
      va_end(ap);
      return result;
    }
  }

  // buffer was too small
  {
    int result;
#ifdef _UNICODE
    result = _vscwprintf(format, ap);
#else
    result = _vscprintf(format, ap);
#endif
    usize maxCount = result + 1;
    tchar* buffer = (tchar*)Memory::alloc(maxCount * sizeof(tchar));
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
  int result = vfprintf(stderr, format, ap);
  va_end(ap);
  return result;
#endif
}

#ifndef NDEBUG
bool Debug::getSourceLine(void* addr, const tchar*& file, int& line)
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
    HMODULE hModule = LoadLibrary(_T("Dbghelp.dll"));
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
