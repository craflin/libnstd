
#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#endif

#include <nstd/Error.h>
#include <nstd/Debug.h>

void_t Error::setLastError(uint_t error)
{
#ifdef _WIN32
  SetLastError((DWORD)error);
#else
  errno = (int)error;
#endif
}

uint_t Error::getLastError()
{
#ifdef _WIN32
  return (uint_t)GetLastError();
#else
  return errno;
#endif
}

String Error::getErrorString(uint_t error)
{
#ifdef _WIN32
  TCHAR errorMessage[256];
  DWORD len = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) errorMessage,
        256, NULL );
  ASSERT(len >= 0 && len <= 256);
  while(len > 0 && String::isSpace(errorMessage[len - 1]))
    --len;
  errorMessage[len] = '\0';
  return String(errorMessage, len);
#else
  const char* errorMessage = strerror(error);
  return String(errorMessage, String::length(errorMessage));
#endif
}
