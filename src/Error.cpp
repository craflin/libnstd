
#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <cstring>
#include <pthread.h>
#endif

#include <nstd/Error.h>
#include <nstd/Map.h>
#include <nstd/Debug.h>

class Error::Private
{
public:
#ifdef _WIN32
  static CRITICAL_SECTION cs;
#else
  static pthread_mutex_t mutex;
#endif
  static Map<uint32_t, String> userErrorStrings;
  static class Framework
  {
  public:
    Framework()
    {
#ifdef _WIN32
      InitializeCriticalSection(&cs);
#else
      pthread_mutexattr_t attr; // TODO: use global var for this?
      pthread_mutexattr_init(&attr);
      VERIFY(pthread_mutex_init(&mutex, &attr) == 0);
#endif
    }
    ~Framework()
    {
#ifdef _WIN32
      DeleteCriticalSection(&cs);
#else
      VERIFY(pthread_mutex_destroy(&mutex) == 0);
#endif
    }
  } framework;
};

#ifdef _WIN32
CRITICAL_SECTION Error::Private::cs;
#else
pthread_mutex_t CRITICAL_SECTION Error::Private::mutex;
#endif
Map<uint32_t, String> Error::Private::userErrorStrings;
Error::Private::Framework Error::Private::framework;

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
  if(error == 0x10000)
  {
    String errorStr;
#ifdef _WIN32
    EnterCriticalSection(&Private::cs);
#else
    VERIFY(pthread_mutex_lock(&Private::mutex) == 0);
#endif

#ifdef _WIN32
    uint32_t threadId = (uint32_t)GetCurrentThreadId();
#else
    uint32_t threadId = (uint32_t)pthread_self();
#endif
    Map<uint32_t, String>::Iterator it = Private::userErrorStrings.find(threadId);
    if(it != Private::userErrorStrings.end())
      errorStr = *it;
#ifdef _WIN32
    LeaveCriticalSection(&Private::cs);
#else
    VERIFY(pthread_mutex_unlock(&Private::mutex) == 0);
#endif
    return errorStr;
  }

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

void_t Error::setErrorString(const String& error)
{
#ifdef _WIN32
  EnterCriticalSection(&Private::cs);
#else
  VERIFY(pthread_mutex_lock(&Private::mutex) == 0);
#endif

#ifdef _WIN32
  uint32_t threadId = (uint32_t)GetCurrentThreadId();
#else
  uint32_t threadId = (uint32_t)pthread_self();
#endif

  String* threadErrorMsg = 0;
  for(Map<uint32_t, String>::Iterator i = Private::userErrorStrings.begin(), end = Private::userErrorStrings.end(), next; i != end; i = next)
  {
    next = i;
    ++next;

#ifdef _WIN32
    HANDLE handle = OpenThread(DELETE, FALSE, i.key());
    if(handle == NULL)
    {
      Private::userErrorStrings.remove(i);
      continue;
    }
    else
      CloseHandle(handle);
#else
    int policy;
    struct sched_param param;
    if(pthread_getschedparam((pthread_t)i.key(), &policy, &param) != 0 && errno == ESRCH)
    {
      Private::userErrorStrings.remove(i);
      continue;
    }
#endif
    if(i.key() == threadId)
      threadErrorMsg = &*i;
  }
  if(threadErrorMsg)
    *threadErrorMsg = error;
  else
    Private::userErrorStrings.insert(threadId, error);
  setLastError(0x10000);
#ifdef _WIN32
  LeaveCriticalSection(&Private::cs);
#else
  VERIFY(pthread_mutex_unlock(&Private::mutex) == 0);
#endif
}
