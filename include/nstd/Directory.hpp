/**
* @file
* Declaration of an object to access a directory.
* @author Colin Graf
*/

#pragma once

#include <nstd/String.hpp>

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
  * @param  [in] dirPath    The path to the directory to be searched.
  * @param  [in] pattern    A search pattern (e.g. "*.inf").
  * @param  [in] dirsOnly   Whether the search should ignore files and should only return directories.
  * @return \c true when the directory was successfully opened. If directory could not be opened, Error::getLastError() can be used for further information on the error.
  */
  bool open(const String& dirPath, const String& pattern = String(), bool dirsOnly = false);

  /**
  * Close the opened directory.
  */
  void close();

  /**
  * Search for the next matching entry in the opened directory.
  * @param  [out] name    The name of the next matching entry.
  * @param  [out] isDir   Whether the entry is a directory.
  * @return \c true when a matching entry was found. \c false indicates that there are no more matching entries or that another error occurred. Error::getLastError() can be used for further information on the error.
  */
  bool read(String& name, bool& isDir);

  /**
  * Check if directory exists.
  * @param  [in] dirPath  The directory.
  * @return \c true if it exists.
  */
  static bool exists(const String& dirPath);

  /**
  * Create a directory. The parent directories in the path are created if they do not exist.
  * @ param [in] dirPath  The path of the directory to be created.
  * @return \c true if the directory was created successfully.
  */
  static bool create(const String& dirPath);

  /**
  * Remove a directory from the file system. If \c recursive is not set to \c true, the function will fail if the directory is not empty.
  * @param  [in] dirPath    The path to the directory to be removed.
  * @param  [in] recursive  Whether the directory should be removed removed recursively.
  * @return \c true when the directory was successfully deleted. If directory was not successfully deleted, Error::getLastError() can be used for further information on the error.
  */
  static bool unlink(const String& dirPath, bool recursive = false);

  /**
  * Remove a directory including its parents. If \c recursive is not set to \c true, the function will fail if the directory is not empty. Parent directories are removed if they would remain empty.
  * @param  [in] dirPath    The path to be removed.
  * @param  [in] recursive  Whether the directory should be removed removed recursively.
  * @return \c true when the directories was successfully deleted. If directory was not successfully deleted, Error::getLastError() can be used for further information on the error. Parent directories may still exist even when this function returned \c true.
  */
  static bool purge(const String& dirPath, bool recursive = false);

  /**
  * Change current working directory.
  * @note This affects all threads of the process.
  * @param  [in] dirPath  The directory to change to. This can be an absolute or relative path.
  * @return \c true if the working directory was changed successfully. If it could not be changed, Error::getLastError() can be used for further information on the error.
  */
  static bool change(const String& dirPath);

  /**
  * Get current working directory.
  * @return The absolute path of the current working directory.
  */
  static String getCurrentDirectory();

  /**
  * Get directory for temporary data.
  * @return The absolute path to the temp directory.
  */
  static String getTempDirectory();

  /**
  * Get the home directory of the current user.
  * @return The absolute path to the home directory.
  */
  static String getHomeDirectory();

private:
  bool dirsOnly;
#ifdef _WIN32
  void* findFile; /**< Win32 FindFirstFile HANDLE */
#ifdef _UNICODE
  char ffd[592]; /**< Buffer for WIN32_FIND_DATA */
#else
  char ffd[320]; /**< Buffer for WIN32_FIND_DATA */
#endif
  bool bufferedEntry; /**< Whether there is a buffered search result in ffd. */
  String dirpath; /**< The path to the directory to be searched. */
  String pattern; /**< A search pattern (e.g. "*.inf") */
#else
  void* dp; /**< Directory descriptor. */
  String dirpath; /**< The path to the directory to be searched. */
  String pattern; /**< A search pattern (e.g. "*.inf"). */
#endif

  Directory(const Directory&);
  Directory& operator=(const Directory&);
};
