/**
* @file Directory.h
* Declaration of a class for accessing directories.
* @author Colin Graf
*/

#pragma once

#include <nstd/String.h>

/** A Class for accessing directories */
class Directory
{
public:

  /** Default constructor */
  Directory();

  /** Destructor */
  ~Directory();

  /**
  * Opens a directory for searching for files in the directory
  * @param dirpath The path to the directory to search in
  * @param pattern A search pattern like "*.inf"
  * @param dirsOnly Search only for directories and ignore files
  * @return Whether the directory was opened successfully
  */
  bool open(const String& dirpath, const String& pattern, bool_t dirsOnly);

  /**
  * Searches the next matching entry in the opened directory
  * @param path The path of the next matching entry
  * @param isDir Whether the next entry is a directory
  * @return Whether a matching entry was found
  */
  bool_t read(String& path, bool_t& isDir);

  static bool_t exists(const String& dir);

  static bool_t create(const String& dir);

  static bool_t remove(const String& dir);

  static bool_t change(const String& dir);

private:
  bool_t dirsOnly;
#ifdef _WIN32
  void_t* findFile; /**< Win32 FindFirstFile HANDLE */
  char_t ffd[320]; /**< Buffer for WIN32_FIND_DATA */
  bool_t bufferedEntry; /**< Whether there is a buffered search result in ffd. */
  String dirpath; /**< The name of the directory. */
  String patternExtension; /**< A search pattern file name extension (e.g. "inf" of "*.inf") */
#else
  void_t* dp; /**< Directory descriptor. */
  String dirpath; /**< The path to the directory to search in */
  String pattern; /**< A search pattern like "*.inf" */
#endif

  Directory(const Directory&);
  Directory& operator=(const Directory&);
};
