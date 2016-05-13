
#ifdef _WIN32
#include <Windows.h>
#ifdef _UNICODE
#include <cstdlib>
#endif
#else
#include <dlfcn.h>
#endif

#include <nstd/Library.h>
#include <nstd/Debug.h>
#ifndef _WIN32
#include <nstd/Error.h>
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

bool_t Library::load(const String& name)
{
#ifdef _WIN32
  if(library)
    return false;
  library = LoadLibrary((const tchar_t*)name);
  return library != 0;
#else
  if(library)
    return false;
  library = dlopen((const char_t*)name, RTLD_NOW | RTLD_GLOBAL);
  if(!library)
  {
    Error::setErrorString(String::fromCString(dlerror()));
    return false;
  }
  return true;
#endif
}

void_t* Library::findSymbol(const String& name)
{
#ifdef _WIN32
#ifdef _UNICODE
  char_t mbname[MAX_PATH];
  size_t destChars;
  if(wcstombs_s(&destChars, mbname, (const tchar_t*)name, name.length()) != 0)
    return 0;
  return (void_t*)GetProcAddress((HMODULE)library, mbname);
#else
  return (void_t*)GetProcAddress((HMODULE)library, (const tchar_t*)name);
#endif
#else
  void_t* sym = dlsym(library, (const tchar_t*)name);
  if(!sym)
  {
    Error::setErrorString(String::fromCString(dlerror()));
    return 0;
  }
  return sym;
#endif
}
