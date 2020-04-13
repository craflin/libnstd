
#pragma once

#include <nstd/String.hpp>

class Error
{
public:
  static void setLastError(uint error);
  static uint getLastError();
  static String getErrorString(uint error = getLastError());
  static void setErrorString(const String& error);

private:
  class Private;
};
