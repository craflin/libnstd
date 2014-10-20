
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

#include <nstd/File.h>
#include <nstd/Debug.h>

File::File()
{
#ifdef _WIN32
  ASSERT(sizeof(void*) >= sizeof(HANDLE));
  fp = INVALID_HANDLE_VALUE;
#else
  fp = 0;
#endif
}

File::~File()
{
  close();
}

bool_t File::open(const String& file, uint_t flags)
{
#ifdef _WIN32
  if(fp != INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return false;
  }
  DWORD desiredAccess = 0, creationDisposition = 0;
  if(flags & writeFlag)
  {
    desiredAccess |= GENERIC_WRITE;
    if(flags & appendFlag)
      creationDisposition = OPEN_ALWAYS;
    else
      creationDisposition = CREATE_ALWAYS;
  }
  if(flags & readFlag)
  {
    desiredAccess |= GENERIC_READ;
    if(!(flags & writeFlag))
      creationDisposition = OPEN_EXISTING;
  }
  fp = CreateFile(file, desiredAccess, FILE_SHARE_READ, NULL, creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
  if(fp == INVALID_HANDLE_VALUE)
    return false;
  if(creationDisposition == OPEN_ALWAYS)
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
  int_t oflags;
  if((flags & (readFlag | writeFlag)) == (readFlag | writeFlag))
    oflags = O_CREAT | O_RDWR; // create if not exists, rw mode
  else if(flags & writeFlag)
  {
    if(flags & appendFlag)
      oflags = O_CREAT | O_WRONLY; // create if not exists, write mode
    else
      oflags = O_CREAT | O_TRUNC | O_WRONLY; // create if not exists, truncate if exist, write mode
  }
  else
    oflags = O_RDONLY; // do not create if not exists, read mode

  fp = (void_t*)(intptr_t)::open(file, oflags | O_CLOEXEC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if((int_t)(intptr_t)fp == -1)
  {
    fp = 0;
    return false;
  }
  if(flags & appendFlag)
  {
    if(lseek((int_t)(intptr_t)fp, 0, SEEK_END) == -1)
    {
      int lastError = errno;
      ::close((int_t)(intptr_t)fp);
      fp = 0;
      errno = lastError;
      return false;
    }
  }
#endif

  return true;
}

void_t File::close()
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
    ::close((int_t)(intptr_t)fp);
    fp = 0;
  }
#endif
}

bool_t File::isOpen() const
{
#ifdef _WIN32
  return fp != INVALID_HANDLE_VALUE;
#else
  return fp != 0;
#endif
}

int64_t File::size()
{
#ifdef _WIN32
  LARGE_INTEGER fs;
  if(!GetFileSizeEx((HANDLE)fp, &fs))
    return -1;
  return fs.QuadPart;
#else
  // todo: use lstat?
  off64_t currentPosition = lseek((int_t)(intptr_t)fp, 0, SEEK_CUR);
  if(currentPosition < 0)
    return -1; 
  off64_t size = lseek((int_t)(intptr_t)fp, 0, SEEK_END);
  if(size < 0)
    return -1;
  if(currentPosition != size)
  {
    currentPosition = lseek((int_t)(intptr_t)fp, currentPosition, SEEK_SET);
    if(currentPosition < 0)
      return -1;
  }
  return size;
#endif
}

bool_t File::unlink(const String& file)
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

ssize_t File::read(void_t* buffer, size_t len)
{
#ifdef _WIN32
#ifdef _AMD64
  byte_t* bufferStart = (byte_t*)buffer;
  DWORD i;
  while(len > (size_t)INT_MAX)
  {
    if(!ReadFile((HANDLE)fp, buffer, INT_MAX, &i, NULL))
      return -1;
    buffer = (byte_t*)buffer + i;
    if(i != INT_MAX)
      return (byte_t*)buffer - bufferStart;
    len -= INT_MAX;
  }
  if(!ReadFile((HANDLE)fp, buffer, (DWORD)len, &i, NULL))
    return -1;
  buffer = (byte_t*)buffer + i;
  return (byte_t*)buffer - bufferStart;
#else
  DWORD i;
  if(!ReadFile((HANDLE)fp, buffer, len, &i, NULL))
    return -1;
  return i;
#endif
#else
  return ::read((int_t)(intptr_t)fp, buffer, len);
#endif
}

bool_t File::readAll(String& data)
{
  int64_t fileSize = size();
  if(fileSize < 0)
    return false;
  data.resize((size_t)fileSize / sizeof(tchar_t));
  size_t dataCount = read((tchar_t*)data, data.length() * sizeof(tchar_t));
  if(dataCount < 0)
  {
    data.clear();
    return false;
  }
  data.resize(dataCount / sizeof(tchar_t));
  return true;
}

ssize_t File::write(const void_t* buffer, size_t len)
{
#ifdef _WIN32
#ifdef _AMD64
  const byte_t* bufferStart = (const byte_t*)buffer;
  DWORD i;
  while(len > (size_t)INT_MAX)
  {
    if(!WriteFile((HANDLE)fp, buffer, INT_MAX, &i, NULL))
      return -1;
    buffer = (const byte_t*)buffer + i;
    if(i != INT_MAX)
      return (const byte_t*)buffer - bufferStart;
    len -= INT_MAX;
  }
  if(!WriteFile((HANDLE)fp, buffer, (DWORD)len, &i, NULL))
    return -1;
  buffer = (const byte_t*)buffer + i;
  return (const byte_t*)buffer - bufferStart;
#else
  DWORD i;
  if(!WriteFile((HANDLE)fp, buffer, len, &i, NULL))
    return -1;
  return i;
#endif
#else
  return ::write((int_t)(intptr_t)fp, buffer, len);
#endif
}

bool_t File::write(const String& data)
{
  size_t size = data.length() * sizeof(tchar_t);
  return write((const byte_t*)(const tchar_t*)data, size) == (ssize_t)size;
}

int64_t File::seek(int64_t offset, Position start)
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
  return lseek64((int_t)(intptr_t)fp, offset, whence[start]);
#endif
}

bool_t File::flush()
{
#ifdef _WIN32
  return FlushFileBuffers((HANDLE)fp) != FALSE;
#else
  return fsync((int_t)(intptr_t)fp) == 0;
#endif
}

String File::dirname(const String& file)
{
  const tchar_t* start = file;
  const tchar_t* pos = &start[file.length() - 1];
  for(; pos >= start; --pos)
    if(*pos == _T('\\') || *pos == _T('/'))
      return file.substr(0, (int_t)(pos - start));
  return String(_T("."));
}

String File::basename(const String& file, const String& extension)
{
  const tchar_t* start = file;
  size_t fileLen = file.length();
  const tchar_t* pos = &start[fileLen - 1];
  const tchar_t* result;
  for(; pos >= start; --pos)
    if(*pos == _T('\\') || *pos == _T('/'))
    {
      result = pos + 1;
      goto removeExtension;
    }
  result = start;
removeExtension:
  size_t resultLen = fileLen - (size_t)(result - start);
  size_t extensionLen = extension.length();
  if(extensionLen)
  {
    const tchar_t* extensionPtr = extension;
    if(*extensionPtr == _T('.'))
    {
      if(resultLen >= extensionLen)
        if(String::compare((const tchar_t*)result + resultLen - extensionLen, extensionPtr) == 0)
          return String(result, resultLen - extensionLen);
    }
    else
    {
      size_t extensionLenPlus1 = extensionLen + 1;
      if(resultLen >= extensionLenPlus1 && result[resultLen - extensionLenPlus1] == _T('.'))
        if(String::compare((const tchar_t*)result + resultLen - extensionLen, extensionPtr) == 0)
          return String(result, resultLen - extensionLenPlus1);
    }
  }
  return String(result, resultLen);
}

String File::extension(const String& file)
{
  const tchar_t* start = file;
  size_t fileLen = file.length();
  const tchar_t* pos = &start[fileLen - 1];
  for(; pos >= start; --pos)
    if(*pos == _T('.'))
      return String(pos + 1, fileLen - ((size_t)(pos - start) + 1));
    else if(*pos == _T('\\') || *pos == _T('/'))
      return String();
  return String();
}

String File::simplifyPath(const String& path)
{
  String result(path.length());
  const tchar_t* data = path;
  const tchar_t* start = data;
  const tchar_t* end;
  const tchar_t* chunck;
  size_t chunckLen;
  bool_t startsWithSlash = *data == _T('/') || *data == _T('\\');
  for(;;)
  {
    while(*start && (*start == _T('/') || *start == _T('\\')))
      ++start;
    end = start;
    while(*end && *end != _T('/') && *end != _T('\\'))
      ++end;

    if(end == start)
      break;

    chunck = start;
    chunckLen = (size_t)(end - start);
    if(chunckLen == 2 && *chunck == _T('.') && chunck[1] == _T('.') && !result.isEmpty())
    {
      const tchar_t* data = result;
      const tchar_t* pos = data + result.length() - 1;
      for(;; --pos)
        if(pos < data || *pos == _T('/') || *pos == _T('\\'))
        {
          if(String::compare(pos + 1, _T("..")) != 0)
          {
            if(pos < data)
              result.resize(0);
            else
              result.resize((size_t)(pos - data));
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
    if(!*end)
      break;
    start = end + 1;
  }
  return result;
}

bool_t File::isAbsolutePath(const String& path)
{
  const tchar_t* data = path;
  return *data == _T('/') || *data == _T('\\') || (path.length() > 2 && data[1] == _T(':') && (data[2] == _T('/') || data[2] == _T('\\')));
}

String File::getRelativePath(const String& from, const String& to)
{
  String simFrom = simplifyPath(from);
  String simTo = simplifyPath(to);
  if(simFrom == simTo)
    return String(_T("."));
  simFrom.append('/');
  if(String::compare((const tchar_t*)simTo, (const tchar_t*)simFrom, simFrom.length()) == 0)
    return String((const tchar_t*)simTo + simFrom.length(), simTo.length() - simFrom.length());
  String result(_T("../"));
  while(simFrom.length() > 0)
  {
    simFrom.resize(simFrom.length() - 1);
    const tchar_t* newEnd = simFrom.findLast('/');
    if(!newEnd)
      break;
    simFrom.resize((newEnd - (const tchar_t*)simFrom) + 1);
    if(String::compare((const tchar_t*)simTo, (const tchar_t*)simFrom, simFrom.length()) == 0)
    {
      result.append(String((const tchar_t*)simTo + simFrom.length(), simTo.length() - simFrom.length()));
      return result;
    }
    result.append(_T("../"));
  }
  return String();
}

bool_t times(const String& file, File::Times& times)
{
#ifdef _WIN32
  WIN32_FIND_DATA wfd;
  HANDLE hFind = FindFirstFile(file, &wfd); // TODO: use another function for this ?
  if(hFind == INVALID_HANDLE_VALUE)
    return false;
  ASSERT(sizeof(DWORD) == 4);
  // TODO: add
  times.writeTime = ((timestamp_t)wfd.ftLastWriteTime.dwHighDateTime << 32LL | (timestamp_t)wfd.ftLastWriteTime.dwLowDateTime) / 10000LL - 11644473600LL;
  times.accessTime = ((timestamp_t)wfd.ftLastAccessTime.dwHighDateTime << 32LL | (timestamp_t)wfd.ftLastAccessTime.dwLowDateTime) / 10000LL - 11644473600LL;
  times.creationTime = ((timestamp_t)wfd.ftCreationTime.dwHighDateTime << 32LL | (timestamp_t)wfd.ftCreationTime.dwLowDateTime) / 10000LL - 11644473600LL;
  FindClose(hFind);
  return true;
#else
  struct stat buf;
  if(stat(file, &buf) != 0)
    return false;
  times.writeTime = ((long long)buf.st_mtim.tv_sec) * 1000LL + ((long long)buf.st_mtim.tv_nsec) / 1000000LL;
  times.accessTime = ((long long)buf.st_atim.tv_sec) * 1000LL + ((long long)buf.st_atim.tv_nsec) / 1000000LL;
  times.creationTime = ((long long)buf.st_ctim.tv_sec) * 1000LL + ((long long)buf.st_ctim.tv_nsec) / 1000000LL;
  return true;
#endif
}

bool_t File::exists(const String& file)
{
#ifdef _WIN32
  WIN32_FIND_DATA wfd;
  HANDLE hFind = FindFirstFile(file, &wfd); // TODO: use GetFileAttribute ?
  if(hFind == INVALID_HANDLE_VALUE)         // I guess GetFileAttribute does not work on network drives. But it will produce
    return false;                           // an error code that can be used to fall back to another implementation.
  FindClose(hFind);
  return true;
#else
  struct stat buf;
  if(lstat(file, &buf) != 0)
    return false;
  return true;
#endif
}
