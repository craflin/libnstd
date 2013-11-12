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

bool Directory::remove(const String& dir)
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

bool Directory::open(const String& dirpath, const String& pattern, bool_t dirsOnly)
{
#ifdef _WIN32
  if(findFile != INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return false;
  }

  this->dirsOnly = dirsOnly;
  this->dirpath = dirpath;
  const tchar_t* patExt = _tcsrchr(pattern, _T('.'));
  this->patternExtension = (patExt && !_tcspbrk(patExt + 1, _T("*?"))) ? String(patExt + 1, -1) : String();

  String searchPath = dirpath;
  searchPath.reserve(dirpath.length() + 1 + pattern.length());
  if(!dirpath.isEmpty())
    searchPath.append(_T('/'));
  searchPath.append(pattern);

  findFile = FindFirstFileEx(searchPath,
#if _WIN32_WINNT > 0x0600
	  FindExInfoBasic,
#else
	  FindExInfoStandard,
#endif
	  (LPWIN32_FIND_DATA)ffd, dirsOnly ? FindExSearchLimitToDirectories : FindExSearchNameMatch, NULL, 0);
  if(findFile == INVALID_HANDLE_VALUE)
    return false;
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

  dp = opendir(dirpath);
  return dp != 0;
#endif
}

bool Directory::read(String& name, bool_t& isDir)
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

    if(!patternExtension.isEmpty())
    {
      const tchar_t* patExt = _tcsrchr(str, _T('.'));
      if(!patExt || _tcsicmp(patternExtension, patExt + 1) != 0)
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
    if(fnmatch(pattern, str, 0) == 0)
    {
      name = String(str, strlen(str));
      isDir = false;
      if(dent->d_type == DT_DIR)
        isDir = true;
      else if(dent->d_type == DT_LNK || dent->d_type == DT_UNKNOWN)
      {
        String path = dirpath;
        path.append(_T('/'));
        path.append(name);
        struct stat buff;
        if(stat(path, &buff) == 0)
          if(S_ISDIR(buff.st_mode))
            isDir = true;
      }
      if(dirsOnly && !isDir)
        continue;
      if(isDir && *str == _T('.') && (str[1] == _T('\0') || (str[1] == _T('.') && str[2] == _T('\0'))))
        continue;
      return true;
    }
  }
  return false; // unreachable
#endif
}

bool Directory::exists(const String& dir)
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

bool Directory::create(const String& dir)
{
  // TODO: set errno correctly

  const tchar_t* start = dir;
  const tchar_t* pos = &start[dir.length() - 1];
  for(; pos >= start; --pos)
    if(*pos == _T('\\') || *pos == _T('/'))
    {
      if(!create(dir.substr(0, (int_t)(pos - start))))
      {
        return false;
      }
      break;
    }
  ++pos;
  bool_t result = false;
  if(*pos)
#ifdef _WIN32
    result = CreateDirectory(dir, NULL) == TRUE;
#else
    result = mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0;
#endif
  return result;
}

bool Directory::change(const String& dir)
{
#ifdef _WIN32
  return SetCurrentDirectory(dir) != FALSE;
#else
  return chdir(dir) == 0;
#endif
}
