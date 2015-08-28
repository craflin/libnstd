/**
* @file Directory.cpp
* Implementation of a class for accessing directories.
* @author Colin Graf
*/

#include <cstring>
#ifdef _WIN32
#include <windows.h>
#ifdef _MSC_VER
#include <tchar.h>
#else
#include <strings.h> // strcasecmp
#endif
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#include <cerrno>
#endif

#if defined(_WIN32) && !defined(_MSC_VER)
#define _tcsrchr strrchr
#define _tcspbrk strpbrk
#define _tcsicmp strcasecmp
#define _tcslen strlen
#endif

#include <nstd/Directory.h>
#include <nstd/File.h>
#include <nstd/Debug.h>

Directory::Directory()
{
#ifdef _WIN32
  //Debug::printf(_T("sizeof(WIN32_FIND_DATA)=%d\n"), sizeof(WIN32_FIND_DATA));
  ASSERT(sizeof(ffd) >= sizeof(WIN32_FIND_DATA));
  findFile = INVALID_HANDLE_VALUE;
#else
  dp = 0;
#endif
}

Directory::~Directory()
{
#ifdef _WIN32
  if(findFile != INVALID_HANDLE_VALUE)
    FindClose((HANDLE)findFile);
#else
  if(dp)
    closedir((DIR*)dp);
#endif
}

bool_t Directory::remove(const String& dir)
{
  String path = dir;
  while(path != _T("."))
  {
#ifdef _WIN32
    if(!RemoveDirectory(path))
      return false;
#else
    if(rmdir(path) != 0)
      return false;
#endif
    path = File::dirname(path);
  }
  return true;
}

bool_t Directory::open(const String& dirpath, const String& pattern, bool_t dirsOnly)
{
#ifdef _WIN32
  if(findFile != INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return false;
  }

  this->dirsOnly = dirsOnly;
  this->dirpath = dirpath;

  // there are three FindFirstFile/FindFirstFileEx issues:
  // 1) '?' does not match exacty 1 character but also 0 characters.
  // 2) pattern machting on file extensions is kinda funky. (*.aaa matches x.aaabb, *a.* matches aa, a?t does not match a.t)
  // 3) it is hard to differentiate between nonexistent directories and directories in which no matching files can be found.

  // solutions:
  // 1) when pattern contains an '?', test if results really match
  // 2) replace '?' with '*', when pattern ends with '.*' or does not end with '*', test if results really match
  // 3) when FindFirstFileEx results in an invalid handle, then try again using '*' as pattern and test if results really match (probably they don't)

  String searchPat = pattern;
  this->pattern.clear();
  if(searchPat.find('?'))
  {
    this->pattern = pattern;
    searchPat.replace('?', '*');
  }
  else if(!pattern.isEmpty() && (pattern[pattern.length() - 1] != '*' || (pattern.length() > 1 && pattern[pattern.length() - 2] == '.')))
    this->pattern = pattern;
  if(searchPat.isEmpty())
    searchPat = _T("*");

  String searchStr = dirpath;
  searchStr.reserve(dirpath.length() + 1 + pattern.length());
  if(!dirpath.isEmpty())
    searchStr.append(_T('/'));
  size_t searchStrLen = searchStr.length();
  searchStr.append(searchPat);

  findFile = FindFirstFileEx(searchStr,
#if _WIN32_WINNT > 0x0600
    FindExInfoBasic,
#else
    FindExInfoStandard,
#endif
    (LPWIN32_FIND_DATA)ffd, dirsOnly ? FindExSearchLimitToDirectories : FindExSearchNameMatch, NULL, 0);
  if(findFile == INVALID_HANDLE_VALUE)
  {
    if(searchPat == _T("*"))
      return false;
    this->pattern = pattern;
    searchStr.resize(searchStrLen);
    searchStr.append(_T("*"));
    findFile = FindFirstFileEx(searchStr,
  #if _WIN32_WINNT > 0x0600
      FindExInfoBasic,
  #else
      FindExInfoStandard,
  #endif
      (LPWIN32_FIND_DATA)ffd, dirsOnly ? FindExSearchLimitToDirectories : FindExSearchNameMatch, NULL, 0);
    if(findFile == INVALID_HANDLE_VALUE)
      return false;
  }
  bufferedEntry = true;
  return true;
#else
  if(dp)
  {
    errno = EINVAL;
    return false;
  }

  this->dirsOnly = dirsOnly;
  this->dirpath = dirpath;
  this->pattern = pattern;

  dp = opendir(dirpath.isEmpty() ? "." : (const char_t*)dirpath);
  return dp != 0;
#endif
}

bool_t Directory::read(String& name, bool_t& isDir)
{
#ifdef _WIN32
  if(!findFile)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return false;
  }
  for(;;)
  {
    if(bufferedEntry)
      bufferedEntry = false;
    else if(!FindNextFile((HANDLE)findFile, (LPWIN32_FIND_DATA)ffd))
    {
      DWORD lastError = GetLastError();
      FindClose((HANDLE)findFile);
      findFile = INVALID_HANDLE_VALUE;
      SetLastError(lastError);
      return false;
    }
    isDir = (((LPWIN32_FIND_DATA)ffd)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
    if(dirsOnly && !isDir)
      continue;
    const tchar_t* str = ((LPWIN32_FIND_DATA)ffd)->cFileName;
    if(isDir && *str == _T('.') && (str[1] == _T('\0') || (str[1] == _T('.') && str[2] == _T('\0'))))
      continue;

    if(!pattern.isEmpty())
    {
      struct PatternMatcher
      {
        static bool szWildMatch7(const tchar_t* pat, const tchar_t* str) {
            const tchar_t* s, * p;
            bool star = false;

        loopStart:
            for (s = str, p = pat; *s; ++s, ++p) {
              switch (*p) {
                  case _T('?'):
                    break;
                  case _T('*'):
                    star = true;
                    str = s, pat = p;
                    do { ++pat; } while (*pat == _T('*'));
                    if (!*pat) return true;
                    goto loopStart;
                  default:
                    if (String::toLowerCase(*s) != String::toLowerCase(*p)) goto starCheck;
                    break;
              }
            }
            while (*p == _T('*')) ++p;
            return !*p;
   
        starCheck:
            if (!star) return false;
            str++;
            goto loopStart;
        }
      };

      if(!PatternMatcher::szWildMatch7(pattern, str))
        continue;
    }

    name = String(str, (uint_t)_tcslen(str));
    return true;
  }
#else
  if(!dp)
  {
    errno = EINVAL;
    return false;
  }

  const char_t* pattern = this->pattern;
  for(;;)
  {
    struct dirent* dent = readdir((DIR*)dp);
    if(!dent)
    {
      int lastErrno = errno;
      closedir((DIR*)dp);
      dp = 0;
      errno = lastErrno;
      return false;
    }
    const char_t* const str = dent->d_name;
    if(!*pattern || fnmatch(pattern, str, 0) == 0)
    {
      isDir = dent->d_type == DT_DIR;
      if(dirsOnly && !isDir)
        continue;
      if(!isDir && (dent->d_type == DT_LNK || dent->d_type == DT_UNKNOWN))
      {
        String path = dirpath;
        if(!dirpath.isEmpty())
          path.append(_T('/'));
        path.append(str, strlen(str));
        struct stat buff;
        if(stat(path, &buff) == 0 && S_ISDIR(buff.st_mode))
          isDir = true;
        else if(dirsOnly)
          continue;
      }
      if(isDir && *str == _T('.') && (str[1] == _T('\0') || (str[1] == _T('.') && str[2] == _T('\0'))))
        continue;
      name = String(str, strlen(str));
      return true;
    }
  }
  return false; // unreachable
#endif
}

bool_t Directory::exists(const String& dir)
{
#ifdef _WIN32
  WIN32_FIND_DATA wfd;
  HANDLE hFind = FindFirstFile(dir, &wfd);
  if(hFind == INVALID_HANDLE_VALUE)
    return false;
  bool_t isDir = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
  FindClose(hFind);
  return isDir;
#else
  struct stat buf;
  if(stat(dir, &buf) != 0)
    return false;
  return S_ISDIR(buf.st_mode);
#endif
}

bool_t Directory::create(const String& dir)
{
  String parent = File::dirname(dir);
  if(parent != _T(".") && !Directory::exists(parent))
  {
    if(!Directory::create(parent))
      return false;
  }
#ifdef _WIN32
  if(!CreateDirectory(dir, NULL))
#else
  if(!mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
#endif
  {
    String basename = File::basename(dir);
    if(basename == _T(".") || basename == _T(".."))
      return true;
  }
  return true;
}

bool_t Directory::unlink(const String& dir)
{
#ifdef _WIN32
  return RemoveDirectory(dir) == TRUE;
#else
  return ::rmdir(dir) == 0;
#endif
}

bool_t Directory::change(const String& dir)
{
#ifdef _WIN32
  return SetCurrentDirectory(dir) != FALSE;
#else
  return ::chdir(dir) == 0;
#endif
}

String Directory::getCurrent()
{
#ifdef _WIN32
  String result;
  result.resize(MAX_PATH);
  DWORD len = GetCurrentDirectory(MAX_PATH + 1, (tchar_t*)result);
  if(len <= MAX_PATH)
  {
    result.resize(len);
    return result;
  }
  result.resize(len);
  DWORD len2 = GetCurrentDirectory(len + 1, (tchar_t*)result);
  if(len2 <= MAX_PATH)
  {
    result.resize(len2);
    return result;
  }
  return String();
#else
  String result;
  result.resize(PATH_MAX);
  for(;;)
  {
    char* success = getcwd((char*)result, result.length());
    if(success)
    {
      result.resize(String::length((char*)result));
      return result;
    }
    if(errno == ERANGE)
    {
      result.resize(result.length() * 2);
      continue;
    }
    return String();
  }

#endif
}
