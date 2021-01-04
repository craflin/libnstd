/**
* @file Directory.cpp
* Implementation of a class for accessing directories.
* @author Colin Graf
*/

#include <cstring>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlobj.h>
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
#include <pwd.h>
#include <cerrno>
#endif

#if defined(_WIN32) && !defined(_MSC_VER)
#define _tcsrchr strrchr
#define _tcspbrk strpbrk
#define _tcsicmp strcasecmp
#define _tcslen strlen
#endif

#include <nstd/Directory.hpp>
#include <nstd/File.hpp>
#include <nstd/Debug.hpp>

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

bool Directory::open(const String& dirpath, const String& pattern, bool dirsOnly)
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
  usize searchStrLen = searchStr.length();
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

  dp = opendir(dirpath.isEmpty() ? "." : (const char*)dirpath);
  return dp != 0;
#endif
}

void Directory::close()
{
#ifdef _WIN32
  if(findFile != INVALID_HANDLE_VALUE)
  {
    FindClose((HANDLE)findFile);
    findFile = INVALID_HANDLE_VALUE;
  }
#else
  if(dp)
  {
    closedir((DIR*)dp);
    dp = 0;
  }
#endif
}

bool Directory::read(String& name, bool& isDir)
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
    const tchar* str = ((LPWIN32_FIND_DATA)ffd)->cFileName;
    if(isDir && *str == _T('.') && (str[1] == _T('\0') || (str[1] == _T('.') && str[2] == _T('\0'))))
      continue;

    if(!pattern.isEmpty())
    {
      struct PatternMatcher
      {
        static bool szWildMatch7(const tchar* pat, const tchar* str) {
            const tchar* s, * p;
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

    name = String(str, (uint)_tcslen(str));
    return true;
  }
#else
  if(!dp)
  {
    errno = EINVAL;
    return false;
  }

  const char* pattern = this->pattern;
  errno = 0;
  for(;;)
  {
    struct dirent* dent = readdir((DIR*)dp);
    if(!dent)
      break;
    const char* const str = dent->d_name;
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
  return false;
#endif
}

bool Directory::exists(const String& dir)
{
#ifdef _WIN32
  WIN32_FIND_DATA wfd;
  HANDLE hFind = FindFirstFile(dir, &wfd);
  if(hFind == INVALID_HANDLE_VALUE)
    return false;
  bool isDir = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
  FindClose(hFind);
  return isDir;
#else
  struct stat buf;
  if(stat(dir, &buf) != 0)
    return false;
  return S_ISDIR(buf.st_mode);
#endif
}

bool Directory::create(const String& dir)
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
  if(mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
#endif
  {
    String basename = File::basename(dir);
    if(basename == _T(".") || basename == _T(".."))
      return true;
  }
  return true;
}

bool Directory::unlink(const String& dir, bool recursive)
{
#ifdef _WIN32
  if(RemoveDirectory(dir))
    return true;
  if(!recursive || GetLastError() != ERROR_DIR_NOT_EMPTY)
    return false;
  WIN32_FIND_DATA ffd;
  HANDLE findFile = FindFirstFileEx(dir + _T("/*"),
#if _WIN32_WINNT > 0x0600
    FindExInfoBasic,
#else
    FindExInfoStandard,
#endif
    &ffd, FindExSearchNameMatch, NULL, 0);
  if(findFile == INVALID_HANDLE_VALUE)
    return false;
  String prefix = dir + _T("/");
  do
  {
    bool isDir = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
    const tchar* str = ffd.cFileName;
    if(isDir && *str == _T('.') && (str[1] == _T('\0') || (str[1] == _T('.') && str[2] == _T('\0'))))
      continue;
    if(isDir)
    {
      if(!unlink(prefix + String(str, String::length(str)), true))
      {
        DWORD err = GetLastError();
        FindClose(findFile);
        SetLastError(err);
        return false;
      }
    }
    else if(!File::unlink(prefix + String(str, String::length(str))))
    {
      DWORD err = GetLastError();
      FindClose(findFile);
      SetLastError(err);
      return false;
    }
  } while(FindNextFile(findFile, &ffd));
  FindClose(findFile);
  return RemoveDirectory(dir) == TRUE;
#else
  if(::rmdir(dir) == 0)
    return true;
  if(!recursive || errno != ENOTEMPTY)
    return false;
  DIR* dp = opendir(dir);
  if(!dp)
    return false;
  String prefix = dir + _T("/");
  errno = 0;
  for(;;)
  {
    struct dirent* dent = readdir(dp);
    if(!dent)
    {
      int lastErrno = errno;
      if(!lastErrno)
        break;
      closedir(dp);
      errno = lastErrno;
      return false;
    }
    const char* const str = dent->d_name;
    bool isDir = dent->d_type == DT_DIR;
    if(isDir && *str == _T('.') && (str[1] == _T('\0') || (str[1] == _T('.') && str[2] == _T('\0'))))
      continue;
    if(isDir)
    {
      if(!unlink(prefix + String(str, String::length(str)), true))
      {
        int lastErrno = errno;
        closedir(dp);
        errno = lastErrno;
        return false;
      }
    }
    else if(!File::unlink(prefix + String(str, String::length(str))))
    {
      int lastErrno = errno;
      closedir(dp);
      errno = lastErrno;
      return false;
    }
  }
  closedir(dp);
  return ::rmdir(dir) == 0;
#endif
}

bool Directory::purge(const String& path, bool recursive)
{
  if(!unlink(path, recursive))
    return false;
  for (String i = File::dirname(path); i != _T("."); i = File::dirname(i))
#ifdef _WIN32
    if(!RemoveDirectory(i))
      break;
#else
    if(rmdir(i) != 0)
      break;
#endif
  return true;
}

bool Directory::change(const String& dir)
{
#ifdef _WIN32
  return SetCurrentDirectory(dir) != FALSE;
#else
  return ::chdir(dir) == 0;
#endif
}

String Directory::getCurrentDirectory()
{
#ifdef _WIN32
  String result;
  result.resize(MAX_PATH);
  DWORD len = GetCurrentDirectory(MAX_PATH + 1, (tchar*)result);
  if(len <= MAX_PATH)
  {
    result.resize(len);
    return result;
  }
  result.resize(len);
  DWORD len2 = GetCurrentDirectory(len + 1, (tchar*)result);
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

String Directory::getTempDirectory()
{
#ifdef _WIN32
  TCHAR buffer[MAX_PATH + 2];
  DWORD len = GetTempPath(MAX_PATH + 2, buffer);
  if(len == 0)
    return String();
  while(len > 1 && buffer[len - 1] == '\\')
    --len;
  return String(buffer, len);
#else
  return String("/tmp");
#endif
}

String Directory::getHomeDirectory()
{
#ifdef _WIN32
    TCHAR path[MAX_PATH + 2];
    if (!SHGetSpecialFolderPath(NULL, path, CSIDL_PROFILE, FALSE))
        return String();
    return String::fromCString(path);
#else
    const struct passwd* pw = getpwuid(geteuid());
    if (!pw)
        return String();
    return String::fromCString(pw->pw_dir);
#endif
}
