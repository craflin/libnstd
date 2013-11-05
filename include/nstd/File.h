
#pragma once

#include <nstd/String.h>

class File
{
public:
  enum Flags
  {
    readFlag = 0x0001,
    writeFlag = 0x0002,
    //appendFlag = 0x0004, // TODO
  };

  struct Times
  {
    timestamp_t writeTime;
    timestamp_t accessTime;
    timestamp_t creationTime;
  };

  File();
  ~File();

  bool_t open(const String& file, uint_t flags = readFlag);
  void_t close();
  int_t read(char_t* buffer, int_t len);
  int_t write(const char_t* buffer, int_t len);
  bool_t write(const String& data);

  static String dirname(const String& file);
  static String basename(const String& file, const String& extension = String());
  static String extension(const String& file);
  static String simplifyPath(const String& path);
  static bool_t isAbsolutePath(const String& path);

  static bool_t times(const String& file, Times& time);

  static bool_t exists(const String& file);
  static bool_t unlink(const String& file);

private:
  void_t* fp;
};
