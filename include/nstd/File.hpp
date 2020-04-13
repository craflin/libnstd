
#pragma once

#include <nstd/String.hpp>

class File
{
public:
  enum Flags
  {
    readFlag = 0x0001,
    writeFlag = 0x0002,
    appendFlag = 0x0004,
    openFlag = 0x0008,
  };

  enum Position
  {
    setPosition,
    currentPosition,
    endPosition,
  };

  struct Time
  {
    int64 writeTime;
    int64 accessTime;
    int64 creationTime;
  };

  File();
  ~File();

  bool open(const String& file, uint flags = readFlag);
  void close();
  bool isOpen() const;

  /**
  * Get size of the file.
  * @return The size of the file in bytes. In case of an error -1 is returned.
  */
  int64 size();

  /**
  * Read a data block from the file at the current read position.
  * The file has to be opened with Flags::readFlag. If successful, the read position changes to after 
  * the block that was read.
  * @param  [in] buffer A buffer where the data should be stored. It has to be large 
  *                     enough to hold \c length bytes.
  * @param  [in] length The count of the bytes to read.
  * @return The amount of bytes that was read. This could be equal 0 or less than \c length when the end of the
  *         file was reached. In case of an error -1 is returned.
  */
  ssize read(void* buffer, usize length);

  /**
  * Read all data from current read position till the end of the file.
  * @param  [out] data  The data.
  * @return Whether the data was successfully read.
  */
  bool readAll(String& data);

  ssize write(const void* buffer, usize length);

  bool write(const String& data);

  /**
  * Move the read or write offset position within the file.
  * @param  [in] offset The offset in bytes from \c start.
  * @param  [in] start  The position from where to start the move operation.
  * @return The position in the file relative to the beginning of the file after the seek operation.
  */
  int64 seek(int64 offset, Position start = setPosition);

  bool flush();

  static String dirname(const String& file); // TODO: rename getDirname()?
  static String basename(const String& file, const String& extension = String()); // TODO: rename getBasename()?
  
  /**
  * Get the extension of a file name or path (without the dot).
  * @param  [in] file The file name or path off which to get the extension.
  */
  static String extension(const String& file); // TODO: rename getExtension()?

  static String simplifyPath(const String& path);
  static bool isAbsolutePath(const String& path);
  static String getRelativePath(const String& from, const String& to);
  static String getAbsolutePath(const String& path);

  static bool time(const String& file, Time& time);

  static bool exists(const String& file);
  static bool unlink(const String& file);
  static bool rename(const String& from, const String& to, bool failIfExists = true);
  static bool copy(const String& src, const String& destination, bool failIfExists = true);
  static bool createSymbolicLink(const String& target, const String& file);

  static bool isExecutable(const String& file);

  static bool readAll(const String& file, String& data);

private:
  void* fp;

  File(const File&);
  File& operator=(const File&);
};
