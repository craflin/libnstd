
#ifdef _WIN32
#ifndef _M_AMD64
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <sys/sendfile.h>
#endif

#include <nstd/File.hpp>
#include <nstd/Debug.hpp>
#include <nstd/Directory.hpp>

File::File()
{
#ifdef _WIN32
  ASSERT(sizeof(fp) >= sizeof(HANDLE));
  fp = INVALID_HANDLE_VALUE;
#else
  fp = 0;
#endif
}

File::~File()
{
  close();
}

bool File::open(const String& file, uint flags)
{
#ifdef _WIN32
  if(fp != INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return false;
  }
  DWORD desiredAccess, creationDisposition;
  if((flags & (readFlag | writeFlag)) == (readFlag | writeFlag))
  {
    desiredAccess = GENERIC_READ | GENERIC_WRITE;
    if(flags & openFlag)
      creationDisposition = OPEN_EXISTING;
    else
      creationDisposition = OPEN_ALWAYS;
  }
  else if(flags & writeFlag)
  {
    desiredAccess = GENERIC_WRITE;
    if(flags & openFlag)
      creationDisposition = OPEN_EXISTING;
    else
    {
      if(flags & appendFlag)
        creationDisposition = OPEN_ALWAYS;
      else
        creationDisposition = CREATE_ALWAYS;
    }
  }
  else
  {
    desiredAccess = GENERIC_READ;
    creationDisposition = OPEN_EXISTING;
  }
  fp = CreateFile(file, desiredAccess, FILE_SHARE_READ, NULL, creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
  if(fp == INVALID_HANDLE_VALUE)
    return false;
  if(flags & appendFlag)
  {
    if(SetFilePointer(fp, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
    {
      DWORD lastError = GetLastError();
      CloseHandle((HANDLE)fp);
      fp = INVALID_HANDLE_VALUE;
      SetLastError(lastError);
      return false;
    }
  }
#else
  if(fp)
  {
    errno = EINVAL;
    return false;
  }
  int oflags;
  if((flags & (readFlag | writeFlag)) == (readFlag | writeFlag))
  {
    if(flags & openFlag)
      oflags = O_RDWR; // fail if not exists, rw mode
    else
      oflags = O_CREAT | O_RDWR; // create if not exists, rw mode
  }
  else if(flags & writeFlag)
  {
    if(flags & openFlag)
      oflags =  O_WRONLY; // fail if not exists, write mode
    else
    {
      if(flags & appendFlag)
        oflags = O_CREAT | O_WRONLY; // create if not exists, write mode
      else
        oflags = O_CREAT | O_TRUNC | O_WRONLY; // create if not exists, truncate if exist, write mode
    }
  }
  else
    oflags = O_RDONLY; // do not create if not exists, read mode

  fp = (void*)(intptr_t)::open(file, oflags | O_CLOEXEC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if((int)(intptr_t)fp == -1)
  {
    fp = 0;
    return false;
  }
  if(flags & appendFlag)
  {
    if(lseek((int)(intptr_t)fp, 0, SEEK_END) == -1)
    {
      int lastError = errno;
      ::close((int)(intptr_t)fp);
      fp = 0;
      errno = lastError;
      return false;
    }
  }
#endif

  return true;
}

void File::close()
{
#ifdef _WIN32
  if(fp != INVALID_HANDLE_VALUE)
  {
    CloseHandle((HANDLE)fp);
    fp = INVALID_HANDLE_VALUE;
  }
#else
  if(fp)
  {
    ::close((int)(intptr_t)fp);
    fp = 0;
  }
#endif
}

bool File::isOpen() const
{
#ifdef _WIN32
  return fp != INVALID_HANDLE_VALUE;
#else
  return fp != 0;
#endif
}

int64 File::size()
{
#ifdef _WIN32
  LARGE_INTEGER fs;
  if(!GetFileSizeEx((HANDLE)fp, &fs))
    return -1;
  return fs.QuadPart;
#else
  // todo: use lstat?
  off64_t currentPosition = lseek((int)(intptr_t)fp, 0, SEEK_CUR);
  if(currentPosition < 0)
    return -1; 
  off64_t size = lseek((int)(intptr_t)fp, 0, SEEK_END);
  if(size < 0)
    return -1;
  if(currentPosition != size)
  {
    currentPosition = lseek((int)(intptr_t)fp, currentPosition, SEEK_SET);
    if(currentPosition < 0)
      return -1;
  }
  return size;
#endif
}

bool File::unlink(const String& file)
{
#ifdef _WIN32
  if(!DeleteFile(file))
    return false;
#else
  if(::unlink(file) != 0)
    return false;
#endif
  return true;
}

bool File::rename(const String& from, const String& to, bool failIfExists)
{
#ifdef _WIN32
  return MoveFileEx(from, to, MOVEFILE_COPY_ALLOWED | (failIfExists ? 0 : MOVEFILE_REPLACE_EXISTING)) == TRUE;
#else
  if(failIfExists)
  {
    int fd = ::open(to, O_CREAT | O_EXCL | O_CLOEXEC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd == -1)
      return false;
    if(::rename(from, to) != 0)
    {
      int err = errno;
      ::close(fd);
      errno = err;
      return false;
    }
    ::close(fd);
    return true;
  }
  else
    return ::rename(from, to) == 0;
#endif
}

bool File::copy(const String& src, const String& destination, bool failIfExists)
{
#ifdef _WIN32
  return CopyFile(src, destination, failIfExists) == TRUE;
#else
    int fd = ::open(src, O_RDONLY);
    if(fd == -1)
      return false;
    off64_t size = lseek(fd, 0, SEEK_END);
    if(size < 0)
      return false;
    if(lseek(fd, 0, SEEK_SET) < 0)
      return false;
    int dest = ::open(destination, failIfExists ? (O_CREAT | O_EXCL | O_CLOEXEC | O_TRUNC | O_WRONLY) : (O_CREAT | O_CLOEXEC | O_TRUNC | O_WRONLY), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(dest == -1)
    {
      ::close(fd);
      return false;
    }
    if(sendfile(dest, fd, 0, size) != size)
    {
      ::close(fd);
      ::close(dest);
      return false;
    }
    ::close(fd);
    ::close(dest);
    return true;
#endif
}

bool File::createSymbolicLink(const String& target, const String& file)
{
#ifdef _WIN32
  return CreateSymbolicLink(file, target, 0) != 0;
#else
  return symlink(target, file) == 0;
#endif
}

ssize File::read(void* buffer, usize len)
{
#ifdef _WIN32
#ifdef _AMD64
  byte* bufferStart = (byte*)buffer;
  DWORD i;
  while(len > (usize)INT_MAX)
  {
    if(!ReadFile((HANDLE)fp, buffer, INT_MAX, &i, NULL))
      return -1;
    buffer = (byte*)buffer + i;
    if(i != INT_MAX)
      return (byte*)buffer - bufferStart;
    len -= INT_MAX;
  }
  if(!ReadFile((HANDLE)fp, buffer, (DWORD)len, &i, NULL))
    return -1;
  buffer = (byte*)buffer + i;
  return (byte*)buffer - bufferStart;
#else
  DWORD i;
  if(!ReadFile((HANDLE)fp, buffer, len, &i, NULL))
    return -1;
  return i;
#endif
#else
  return ::read((int)(intptr_t)fp, buffer, len);
#endif
}

bool File::readAll(String& data)
{
  int64 fileSize = size();
  if(fileSize < 0)
    return false;
  data.resize((usize)fileSize / sizeof(tchar));
  ssize dataCount = read((tchar*)data, data.length() * sizeof(tchar));
  if(dataCount < 0)
  {
    data.clear();
    return false;
  }
  data.resize(dataCount / sizeof(tchar));
  return true;
}

ssize File::write(const void* buffer, usize len)
{
#ifdef _WIN32
#ifdef _AMD64
  const byte* bufferStart = (const byte*)buffer;
  DWORD i;
  while(len > (usize)INT_MAX)
  {
    if(!WriteFile((HANDLE)fp, buffer, INT_MAX, &i, NULL))
      return -1;
    buffer = (const byte*)buffer + i;
    if(i != INT_MAX)
      return (const byte*)buffer - bufferStart;
    len -= INT_MAX;
  }
  if(!WriteFile((HANDLE)fp, buffer, (DWORD)len, &i, NULL))
    return -1;
  buffer = (const byte*)buffer + i;
  return (const byte*)buffer - bufferStart;
#else
  DWORD i;
  if(!WriteFile((HANDLE)fp, buffer, len, &i, NULL))
    return -1;
  return i;
#endif
#else
  return ::write((int)(intptr_t)fp, buffer, len);
#endif
}

bool File::write(const String& data)
{
  usize size = data.length() * sizeof(tchar);
  return write((const byte*)(const tchar*)data, size) == (ssize)size;
}

int64 File::seek(int64 offset, Position start)
{
#ifdef _WIN32
  DWORD moveMethod[3] = {FILE_BEGIN, FILE_CURRENT, FILE_END};
#ifndef _AMD64
  LARGE_INTEGER li;
  li.QuadPart = offset;
  li.LowPart = SetFilePointer((HANDLE)fp, li.LowPart, &li.HighPart, moveMethod[start]);
  if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
    return -1;
  return li.QuadPart;
#else
  return SetFilePointer((HANDLE)fp, (LONG)offset, NULL, moveMethod[start]);
#endif
#else
  int whence[3] = {SEEK_SET, SEEK_CUR, SEEK_END};
  return lseek64((int)(intptr_t)fp, offset, whence[start]);
#endif
}

bool File::flush()
{
#ifdef _WIN32
  return FlushFileBuffers((HANDLE)fp) != FALSE;
#else
  return fsync((int)(intptr_t)fp) == 0;
#endif
}

String File::dirname(const String& file)
{
  const tchar* start = file;
  const tchar* pos = &start[file.length() - 1];
  for(; pos >= start; --pos)
    if(*pos == _T('\\') || *pos == _T('/'))
      return file.substr(0, pos - start);
  return String(_T("."));
}

String File::basename(const String& file, const String& extension)
{
  const tchar* start = file;
  usize fileLen = file.length();
  const tchar* pos = &start[fileLen - 1];
  const tchar* result;
  for(; pos >= start; --pos)
    if(*pos == _T('\\') || *pos == _T('/'))
    {
      result = pos + 1;
      goto removeExtension;
    }
  result = start;
removeExtension:
  usize resultLen = fileLen - (usize)(result - start);
  usize extensionLen = extension.length();
  if(extensionLen)
  {
    const tchar* extensionPtr = extension;
    if(*extensionPtr == _T('.'))
    {
      if(resultLen >= extensionLen)
        if(String::compare((const tchar*)result + resultLen - extensionLen, extensionPtr) == 0)
          return String(result, resultLen - extensionLen);
    }
    else
    {
      usize extensionLenPlus1 = extensionLen + 1;
      if(resultLen >= extensionLenPlus1 && result[resultLen - extensionLenPlus1] == _T('.'))
        if(String::compare((const tchar*)result + resultLen - extensionLen, extensionPtr) == 0)
          return String(result, resultLen - extensionLenPlus1);
    }
  }
  return String(result, resultLen);
}

String File::extension(const String& file)
{
  const tchar* start = file;
  usize fileLen = file.length();
  const tchar* pos = &start[fileLen - 1];
  for(; pos >= start; --pos)
    if(*pos == _T('.'))
      return String(pos + 1, fileLen - ((usize)(pos - start) + 1));
    else if(*pos == _T('\\') || *pos == _T('/'))
      return String();
  return String();
}

String File::simplifyPath(const String& path)
{
  String result(path.length());
  const tchar* data = path;
  const tchar* start = data;
  const tchar* startEnd = start + path.length();
  const tchar* end;
  const tchar* chunck;
  usize chunckLen;
  bool startsWithSlash = *data == _T('/') || *data == _T('\\');
  for(;;)
  {
    while(start < startEnd && (*start == _T('/') || *start == _T('\\')))
      ++start;
    end = start;
    while(end < startEnd && *end != _T('/') && *end != _T('\\'))
      ++end;

    if(end == start)
      break;

    chunck = start;
    chunckLen = (usize)(end - start);
    if(chunckLen == 2 && *chunck == _T('.') && chunck[1] == _T('.') && !result.isEmpty())
    {
      const tchar* data = result;
      const tchar* pos = data + result.length() - 1;
      for(;; --pos)
        if(pos < data || *pos == _T('/') || *pos == _T('\\'))
        {
          if(String::compare(pos + 1, _T("..")) != 0)
          {
            if(pos < data)
              result.resize(0);
            else
              result.resize((usize)(pos - data));
            goto cont;
          }
          break;
        }
    }
    else if(chunckLen == 1 && *chunck == _T('.'))
      goto cont;

    if(!result.isEmpty() || startsWithSlash)
      result.append(_T('/'));
    result.append(chunck, chunckLen);

  cont:
    if(end >= startEnd)
      break;
    start = end + 1;
  }
  return result;
}

bool File::isAbsolutePath(const String& path)
{
  const tchar* data = path;
  return *data == _T('/') || *data == _T('\\') || (path.length() > 2 && data[1] == _T(':') && (data[2] == _T('/') || data[2] == _T('\\')));
}

String File::getRelativePath(const String& from, const String& to)
{
  String simFrom = simplifyPath(from);
  String simTo = simplifyPath(to);
  if(simFrom == simTo)
    return String(_T("."));
  simFrom.append(_T('/'));
  if(String::compare((const tchar*)simTo, (const tchar*)simFrom, simFrom.length()) == 0)
    return String((const tchar*)simTo + simFrom.length(), simTo.length() - simFrom.length());
  String result(_T("../"));
  while(simFrom.length() > 0)
  {
    simFrom.resize(simFrom.length() - 1);
    const tchar* newEnd = simFrom.findLast(_T('/'));
    if(!newEnd)
      break;
    simFrom.resize((newEnd - (const tchar*)simFrom) + 1);
    if(String::compare((const tchar*)simTo, (const tchar*)simFrom, simFrom.length()) == 0)
    {
      result.append(String((const tchar*)simTo + simFrom.length(), simTo.length() - simFrom.length()));
      return result;
    }
    result.append(_T("../"));
  }
  return String();
}

String File::getAbsolutePath(const String& path)
{
  if(isAbsolutePath(path))
    return path;
  String result = Directory::getCurrentDirectory();
  result.reserve(result.length() + 1 + path.length());
  result.append('/');
  result.append(path);
  return result;
}

bool File::time(const String& file, File::Time& time)
{
#ifdef _WIN32
  WIN32_FIND_DATA wfd;
  HANDLE hFind = FindFirstFileEx(file, // FindFirstFile is faster than GetFileAttribute
#if _WIN32_WINNT > 0x0600
    FindExInfoBasic,
#else
    FindExInfoStandard,
#endif
    &wfd, FindExSearchNameMatch, NULL, 0);
  if(hFind == INVALID_HANDLE_VALUE)
    return false;
  ASSERT(sizeof(DWORD) == 4);
  time.writeTime = ((ULARGE_INTEGER&)wfd.ftLastWriteTime).QuadPart / 10000LL - 11644473600LL* 1000LL;
  time.accessTime = ((ULARGE_INTEGER&)wfd.ftLastAccessTime).QuadPart / 10000LL - 11644473600LL * 1000LL;
  time.creationTime = ((ULARGE_INTEGER&)wfd.ftCreationTime).QuadPart  / 10000LL - 11644473600LL * 1000LL;
  FindClose(hFind);
  return true;
#else
  struct stat buf;
  if(stat(file, &buf) != 0)
    return false;
  time.writeTime = ((long long)buf.st_mtim.tv_sec) * 1000LL + ((long long)buf.st_mtim.tv_nsec) / 1000000LL;
  time.accessTime = ((long long)buf.st_atim.tv_sec) * 1000LL + ((long long)buf.st_atim.tv_nsec) / 1000000LL;
  time.creationTime = ((long long)buf.st_ctim.tv_sec) * 1000LL + ((long long)buf.st_ctim.tv_nsec) / 1000000LL;
  return true;
#endif
}

bool File::exists(const String& file)
{
#ifdef _WIN32
  WIN32_FIND_DATA wfd;
  HANDLE hFind = FindFirstFileEx(file, // FindFirstFile is faster than GetFileAttribute
#if _WIN32_WINNT > 0x0600
    FindExInfoBasic,
#else
    FindExInfoStandard,
#endif
    &wfd, FindExSearchNameMatch, NULL, 0);
  if(hFind == INVALID_HANDLE_VALUE)
    return false;
  FindClose(hFind);
  return true;
#else
  struct stat buf;
  if(lstat(file, &buf) != 0)
    return false;
  return true;
#endif
}

bool File::isExecutable(const String& file)
{
#ifdef _WIN32
  String extension = File::extension(file).toLowerCase();
  return extension == _T("exe") || extension == _T("com") || extension == _T("bat");
#else
  struct stat buf;
  if(stat(file, &buf) != 0)
    return false;
  return (buf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0;
#endif
}

bool File::readAll(const String& path, String& data)
{
  File file;
  if(!file.open(path))
    return false;
  return file.readAll(data);
}
