/**
* @file Directory.h
*
* Declaration of an object to access a directory.
*
* @author Colin Graf
*/

#pragma once

#include <nstd/String.h>

/** An object to access a directory. */
class Directory
{
public:

  /** Default constructor. */
  Directory();

  /** Destructor. */
  ~Directory();

  /**
  * Open a directory to search for files in this directory.
  *
  * @param  [in] dirPath    The path to the directory to be searched.
  * @param  [in] pattern    A search pattern (e.g. "*.inf").
  * @param  [in] dirsOnly   Whether the search should ignore files and should only return directories.
  *
  * @return \c true when the directory was successfully opened. If directory could not be opened, Error::getLastError() can be used for further information on the error.
  */
  bool_t open(const String& dirPath, const String& pattern, bool_t dirsOnly);

  /**
  * Search for the next matching entry in the opened directory.
  *
  * @param  [out] name    The name of the next matching entry.
  * @param  [out] isDir   Whether the entry is a directory.
  *
  * @return \c true when a matching entry was found. \c false indicates that there are no more matching entries or that another error occurred. Error::getLastError() can be used for further information on the error.
  */
  bool_t read(String& name, bool_t& isDir);

  /**
  * Check if directory exists.
  *
  * @param  [in] dirPath  The directory.
  *
  * @return \c true if it exists.
  */
  static bool_t exists(const String& dirPath);

  /**
  * Create a directory. The parent directories in the path are created as well if they do not exist.
  *
  * @ param [in] dirPath  The path of the directory to be created.
  *
  * @return \c true if the directory was created successfully.
  */
  static bool_t create(const String& dirPath);

  /**
  * Remove directory from file system. The directory has to be empty.
  *
  * @param  [in] dirPath  The path to the directory.
  *
  * @return \c true when the directory was successfully deleted. If directory was not successfully deleted, Error::getLastError() can be used for further information on the error.
  */
  static bool_t unlink(const String& dirPath);

  /**
  * Remove directory including its parents. The directory has to be empty. The parent directories have to after its child has been removed.
  *
  * @param  [in] dirPath   The path to be removed.
  *
  * @return \c true when all directories of the path were removed successfully. If directory was not successfully deleted, Error::getLastError() can be used for further information on the error.
  */
  static bool_t unlinkAll(const String& dirPath);

  /**
  * Change current working directory.
  * @note This affects all threads of the process.
  *
  * @param  [in] dirPath  The directory to change to. This can be an absolute or relative path.
  *
  * @return \c true if the working directory was changed successfully. If it could not be changed, Error::getLastError() can be used for further information on the error.
  */
  static bool_t change(const String& dirPath);

  /**
  * Get current working directory.
  *
  * @return The absolute path of the current working directory.
  */
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
