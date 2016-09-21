
#pragma once

#include <nstd/Variant.h>

class JSON
{
public:
  class Parser
  {
  public:
    Parser();
    ~Parser();

    int_t getErrorLine() const;
    int_t getErrorColumn() const;
    String getErrorString() const;

    bool_t parse(const tchar_t* data, Variant& result);
    bool_t parse(const String& data, Variant& result);

  private:
    class Private;
    Private* p;
  };

public:
  static bool_t parse(const tchar_t* data, Variant& result);
  static bool_t parse(const String& data, Variant& result);

  static String toString(const Variant& data);

private:
  class Private;
};
