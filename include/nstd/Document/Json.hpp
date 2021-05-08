
#pragma once

#include <nstd/Variant.hpp>

class Json
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
  /**
   * Strips C/C++ style comments from JSON data without affecting the line in which data is declared.
   *
   * @param [in] data The JSON data.
   * @return  The JSON data with stripped comments.
   */
  static String stripComments(const String& data);

  static bool parse(const tchar* data, Variant& result);
  static bool parse(const String& data, Variant& result);

  static String toString(const Variant& data);

private:
  class Private;
};
