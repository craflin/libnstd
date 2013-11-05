/**
* @file Directory.cpp
* Implementation of a class for accessing directories.
* @author Colin Graf
*/

#include <nstd/Directory.h>
#include <nstd/File.h>
#include <nstd/Debug.h>

#include <cstring>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#include <cerrno>
#endif

#if _MSC_VER
#define stricmp _stricmp
#endif

Directory::Directory()
{
#ifdef _WIN32
  findFile = INVALID_HANDLE_VALUE;
  ASSERT(sizeof(ffd) >= sizeof(WIN32_FIND_DATA));
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
  while(path != ".")
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
  const char* patExt = strrchr(pattern, '.');
  this->patternExtension = (patExt && !strpbrk(patExt + 1, "*?")) ? String(patExt + 1, -1) : String();

  String searchPath = dirpath;
  searchPath.reserve(dirpath.length() + 1 + pattern.length());
  if(!dirpath.isEmpty())
    searchPath.append('/');
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
    const char* str = ((LPWIN32_FIND_DATA)ffd)->cFileName;
    if(isDir && *str == '.' && (str[1] == '\0' || (str[1] == '.' && str[2] == '\0')))
      continue;

    if(!patternExtension.isEmpty())
    {
      const char* patExt = strrchr(str, '.');
      if(!patExt || stricmp(patternExtension, patExt + 1) != 0)
        continue;
    }

    name = String(str, strlen(str));
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
    const char* const str = dent->d_name;
    if(fnmatch(pattern, str, 0) == 0)
    {
      name = String(str, strlen(str));
      isDir = false;
      if(dent->d_type == DT_DIR)
        isDir = true;
      else if(dent->d_type == DT_LNK || dent->d_type == DT_UNKNOWN)
      {
        String path = dirpath;
        path.append('/');
        path.append(name);
        struct stat buff;
        if(stat(path, &buff) == 0)
          if(S_ISDIR(buff.st_mode))
            isDir = true;
      }
      if(dirsOnly && !isDir)
        continue;
      if(isDir && *str == '.' && (str[1] == '\0' || (str[1] == '.' && str[2] == '\0')))
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
  WIN32_FIND_DATAA wfd;
  HANDLE hFind = FindFirstFileA(dir, &wfd);
  if(hFind == INVALID_HANDLE_VALUE)
    return false;
  bool isDir = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
  FindClose(hFind);
  return isDir;
#else
  struct stat buf;
  if(stat(dir.getData(), &buf) != 0)
    return false;
  return S_ISDIR(buf.st_mode);
#endif
}

bool Directory::create(const String& dir)
{
  // TODO: set errno correctly

  const char* start = dir;
  const char* pos = &start[dir.length() - 1];
  for(; pos >= start; --pos)
    if(*pos == '\\' || *pos == '/')
    {
      if(!create(dir.substr(0, pos - start)))
      {
        return false;
      }
      break;
    }
  ++pos;
  bool result = false;
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
