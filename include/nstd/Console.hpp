
#pragma once

#include <nstd/String.hpp>

class Console
{
public:
  static int print(const char* str);
  static int printf(const char* format, ...);
  static int error(const char* str);
  static int errorf(const char* format, ...);

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
