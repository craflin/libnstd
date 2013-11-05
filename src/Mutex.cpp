
#include <nstd/Mutex.h>
#include <nstd/Debug.h>

#ifdef _WIN32
#include <Windows.h>
#else
#endif

Mutex::Mutex()
{
#ifdef _WIN32
  ASSERT(sizeof(data) >= sizeof(CRITICAL_SECTION));
  InitializeCriticalSection(&(CRITICAL_SECTION&)data);
#else
#warning todo
#endif
}

Mutex::~Mutex()
{
#ifdef _WIN32
  DeleteCriticalSection(&(CRITICAL_SECTION&)data);
#else
#warning todo
#endif
}

void_t Mutex::lock()
{
#ifdef _WIN32
  EnterCriticalSection(&(CRITICAL_SECTION&)data);
#else
#warning todo
#endif
}

bool_t Mutex::tryLock()
{
#ifdef _WIN32
  return TryEnterCriticalSection(&(CRITICAL_SECTION&)data) == TRUE;
#else
#warning todo
#endif
}

void_t Mutex::unlock()
{
#ifdef _WIN32
  LeaveCriticalSection(&(CRITICAL_SECTION&)data);
#else
#warning todo
#endif
}
