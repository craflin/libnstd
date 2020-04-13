
#pragma once

#include <nstd/String.hpp>

class Library
{
public:
  Library();
  ~Library();

  bool load(const String& name);

  void* findSymbol(const String& name);

private:
  void* library;

  Library(const Library&);
  Library& operator=(const Library&);
};
