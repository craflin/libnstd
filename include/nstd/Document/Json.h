
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

    int getErrorLine() const;
    int getErrorColumn() const;
    String getErrorString() const;

    bool parse(const tchar* data, Variant& result);
    bool parse(const String& data, Variant& result);

  private:
    class Private;
    Private* p;
  };

public:
  static bool parse(const tchar* data, Variant& result);
  static bool parse(const String& data, Variant& result);

  static String toString(const Variant& data);

private:
  class Private;
};
