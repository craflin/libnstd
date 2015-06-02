/**
* @file Directory.h
*
* Declaration of a class to access a directory.
*
* @author Colin Graf
*/

#pragma once

#include <nstd/String.h>

/** A Class to access a directory. */
class Directory
{
public:

  /** Default constructor. */
  Directory();

  /** Destructor. */
  ~Directory();

  /**
  * Open a directory to search for files in the directory.
  *
  * @param  [in] dirpath  The path to the directory to be searched.
  * @param  [in] pattern  A search pattern (e.g. "*.inf").
  * @param  [in] dirsOnly Whether the search should ignore files and should only include directories.
  *
  * @return \c true when the directory was successfully opened.
  */
  bool_t open(const String& dirpath, const String& pattern, bool_t dirsOnly);

  /**
  * Search for the next matching entry in the opened directory.
  *
  * @param  [out] name  The name of the next matching entry.
  * @param  [out] isDir Whether the entry is a directory or not.
  *
  * @return \c true when matching entry was found.
  */
  bool_t read(String& name, bool_t& isDir);

  static bool_t exists(const String& dir);

  static bool_t create(const String& dir);

  static bool_t remove(const String& dir);

  /**
  * Remove directory from file system. The directory has to be empty.
  *
  * @param  [in] dir  The path to the directory.
  *
  * @return \c true when the directory was successfully deleted. If directory was not successfully deleted, Error::getLastError() can be used for further information on the error.
  */
  static bool_t unlink(const String& dir);

  static bool_t change(const String& dir);

  static String getCurrent();

private:
  bool_t dirsOnly;
#ifdef _WIN32
  void_t* findFile; /**< Win32 FindFirstFile HANDLE */
#ifdef _UNICODE
  char_t ffd[592]; /**< Buffer for WIN32_FIND_DATA */
#else
  char_t ffd[320]; /**< Buffer for WIN32_FIND_DATA */
#endif
  bool_t bufferedEntry; /**< Whether there is a buffered search result in ffd. */
  String dirpath; /**< The path to the directory to be searched. */
  String pattern; /**< A search pattern (e.g. "*.inf") */
#else
  void_t* dp; /**< Directory descriptor. */
  String dirpath; /**< The path to the directory to be searched. */
  String pattern; /**< A search pattern (e.g. "*.inf"). */
#endif

  Directory(const Directory&);
  Directory& operator=(const Directory&);
};
