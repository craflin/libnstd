
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#ifdef _UNICODE
#include <cstdlib>
#endif
#else
#include <dlfcn.h>
#endif

#include <nstd/Library.hpp>
#include <nstd/Debug.hpp>
#ifndef _WIN32
#include <nstd/Error.hpp>
#endif

Library::Library() : library(0) {}

Library::~Library()
{
  if(library)
  {
#ifdef _WIN32
    VERIFY(FreeLibrary((HMODULE)library));
#else
    VERIFY(dlclose(library) == 0);
#endif
  }
}

bool Library::load(const String& name)
{
#ifdef _WIN32
  if(library)
    return false;
  library = LoadLibrary((const tchar*)name);
  return library != 0;
#else
  if(library)
    return false;
  library = dlopen((const char*)name, RTLD_NOW | RTLD_GLOBAL);
  if(!library)
  {
    Error::setErrorString(String::fromCString(dlerror()));
    return false;
  }
  return true;
#endif
}

void* Library::findSymbol(const String& name)
{
#ifdef _WIN32
#ifdef _UNICODE
  char mbname[MAX_PATH];
  usize destChars;
  if(wcstombs_s(&destChars, mbname, (const tchar*)name, name.length()) != 0)
    return 0;
  return (void*)GetProcAddress((HMODULE)library, mbname);
#else
  return (void*)GetProcAddress((HMODULE)library, (const tchar*)name);
#endif
#else
  void* sym = dlsym(library, (const tchar*)name);
  if(!sym)
  {
    Error::setErrorString(String::fromCString(dlerror()));
    return 0;
  }
  return sym;
#endif
}
