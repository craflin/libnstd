
#pragma once

#include <nstd/String.h>

class Console
{
public:
  static int_t print(const tchar_t* str);
  static int_t printf(const tchar_t* format, ...);
  static int_t error(const tchar_t* str);
  static int_t errorf(const tchar_t* format, ...);

  class Prompt
  {
  public:
    Prompt();
    ~Prompt();

    String getLine(const String& prompt);

  private:
    class Private;
    Private* p;
  };
};
