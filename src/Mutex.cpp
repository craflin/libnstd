
#include <nstd/Mutex.h>
#include <nstd/Debug.h>

#include <Windows.h>

Mutex::Mutex()
{
  ASSERT(sizeof(data) >= sizeof(CRITICAL_SECTION));
  InitializeCriticalSection(&(CRITICAL_SECTION&)data);
}

Mutex::~Mutex()
{
  DeleteCriticalSection(&(CRITICAL_SECTION&)data);
}

void_t Mutex::lock()
{
  EnterCriticalSection(&(CRITICAL_SECTION&)data);
}

bool_t Mutex::tryLock()
{
  return TryEnterCriticalSection(&(CRITICAL_SECTION&)data) == TRUE;
}

void_t Mutex::unlock()
{
  LeaveCriticalSection(&(CRITICAL_SECTION&)data);
}
