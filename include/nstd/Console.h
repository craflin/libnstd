
#pragma once

#include <nstd/Base.h>

class Console
{
public:
  static int_t print(const char_t* str);
  static int_t printf(const char_t* format, ...);
};
