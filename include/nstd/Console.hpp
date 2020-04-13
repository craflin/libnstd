
#pragma once

#include <nstd/String.hpp>

class Console
{
public:
  static int print(const tchar* str);
  static int printf(const tchar* format, ...);
  static int error(const tchar* str);
  static int errorf(const tchar* format, ...);

  class Prompt
  {
  public:
    Prompt();
    ~Prompt();

    String getLine(const String& prompt);

  private:
    Prompt(const Prompt&);
    Prompt& operator=(const Prompt&);

  private:
    class Private;
    Private* p;
  };
};
