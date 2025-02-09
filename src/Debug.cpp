
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#ifndef NDEBUG
#include <Dbghelp.h>
#endif
#ifdef UNICODE
#include <cstdlib>
#endif
#else
#ifndef NDEBUG
#include <cstdlib>
#include <execinfo.h>
#include <nstd/Process.hpp>
#endif
#endif
#include <cstdio>
#include <cstdarg>

#include <nstd/Debug.hpp>
#include <nstd/Memory.hpp>
#include <nstd/String.hpp>

int Debug::print(const char* str)
{
#ifdef _MSC_VER
  OutputDebugString(str);
  return (int)strlen(str);
#else
  return fputs(str, stderr);
#endif
}

int Debug::printf(const char* format, ...)
{
#ifdef _MSC_VER
  va_list ap;
  va_start(ap, format);
  {
    char buffer[4096];
    int result = vsnprintf(buffer, sizeof(buffer), format, ap);
    if(result >= 0 && result < (int)(sizeof(buffer)))
    {
      OutputDebugString(buffer);
      va_end(ap);
      return result;
    }
  }

  // buffer was too small
  {
    int result = _vscprintf(format, ap);
    usize maxCount = result + 1;
    char* buffer = (char*)new char[maxCount];
    result = vsnprintf(buffer, maxCount, format, ap);
    va_end(ap);
    OutputDebugString(buffer);
    delete[] (char*)buffer;
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
bool Debug::getSourceLine(void* addr, String& file, int& line)
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
  size_t len = mbstowcs(fileName, ihLine.FileName, strlen(ihLine.FileName));
  if(len == -1)
      return false;
  file = String(fileName, len);
#else
  file = String::fromCString(ihLine.FileName);
#endif
  line = ihLine.LineNumber;
  return true;
#else
  void* addrs[1];
  addrs[0] = addr;
  char** addrStrs = backtrace_symbols(addrs, 1);
  if(!addrStrs)
      return false;
  if(!*addrStrs)
  {
      free(addrStrs);
      return false;
  }
  String addrStr = String::fromCString(*addrStrs);
  free(addrStrs);
  const char* addrStart = addrStr.findLast('[');
  const char* addrEnd = addrStr.findLast(']');
  if(!addrStart ||!addrEnd || addrEnd < addrStart)
      return false;
  ++addrStart;
  void* relAddr;
  if(addrStr.substr(addrStart - (const char*)addrStr, addrEnd - addrStart).scanf("%p", &relAddr) != 1)
      return false;
  const char* binaryEnd = addrStr.findLast('(');
  if(!binaryEnd)
      return false;
  String bin = addrStr.substr(0, binaryEnd - (const char*)addrStr);
  String cmd = String::fromPrintf("addr2line -e \"%s\" %p", (const char*)bin, relAddr);
  Process process;
  if(!process.open(cmd))
      return false;
  String buf;
  buf.reserve(1024 * 32);
  ssize i = process.read((char*)buf, 1024 * 32);
  if(i < 0)
    return false;
  buf.resize(i);
  const char* fileEnd = buf.findLast(':');
  if(!fileEnd)
      return false;
  file = buf.substr(0, fileEnd - (const char*)buf);
  if(buf.substr(fileEnd - (const char*)buf + 1).scanf("%d", &line) != 1)
      return false;
  return true;
#endif
}
#endif
