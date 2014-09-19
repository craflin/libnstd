
#pragma once

#include <nstd/String.h>

class Library
{
public:
  Library();
  ~Library();

  bool_t load(const String& name);

  void_t* findSymbol(const String& name);

  String getErrorString();

private:
  void_t* library;

  Library(const Library&);
  Library& operator=(const Library&);
};
