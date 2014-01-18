
#pragma once

#include <nstd/String.h>

class Error
{
public:
  static void_t setLastError(uint_t error);
  static uint_t getLastError();
  static String getErrorString(uint_t error = getLastError());
};
