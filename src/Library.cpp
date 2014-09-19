
#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include <nstd/Library.h>
#include <nstd/Debug.h>
#ifdef _WIN32
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
  library = dlopen((const char_t*)name, RTLD_NOW);
  return library != 0;
#endif
}

void_t* Library::findSymbol(const String& name)
{
#ifdef _WIN32
  return GetProcAddress((HMODULE)library, (const tchar_t*)name);
#else
  return dlsym(library, (const tchar_t*)name);
#endif
}

String Library::getErrorString()
{
#ifdef _WIN32
  return Error::getErrorString();
#else
  const tchar_t* error = dlerror();
  return String(error, String::length(error));
#endif
}
