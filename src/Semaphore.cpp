/**
* Implementation of a Semaphore object.
* @author Colin Graf
*/

#ifdef _WIN32
#include <climits>
#include <windows.h>
#else
#include <semaphore.h>
#include <ctime>
#endif

#include <nstd/Semaphore.h>
#include <nstd/Debug.h>

Semaphore::Semaphore(unsigned int value)
{
#ifdef _WIN32
  VERIFY(handle = CreateSemaphore(NULL, value, LONG_MAX, NULL));
#else
  ASSERT(sizeof(sem_t) >= sizeof(handle));
  VERIFY(sem_init((sem_t*)&handle, 0, value) != -1);
#endif
}

Semaphore::~Semaphore()
{
#ifdef _WIN32
  VERIFY(CloseHandle(handle));
#else
  VERIFY(sem_destroy((sem_t*)&handle) != -1);
#endif
}

void Semaphore::post()
{
#ifdef _WIN32
  VERIFY(ReleaseSemaphore((HANDLE)handle, 1, 0));
#else
  VERIFY(sem_post((sem_t*)&handle) != -1);
#endif
}

bool Semaphore::wait()
{
#ifdef _WIN32
  return WaitForSingleObject((HANDLE)handle, INFINITE) == WAIT_OBJECT_0;
#else
  return sem_wait((sem_t*)&handle) != -1;
#endif
}

bool Semaphore::wait(timestamp_t timeout)
{
#ifdef _WIN32
  return WaitForSingleObject((HANDLE)handle, (DWORD)timeout) == WAIT_OBJECT_0;
#else
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_nsec += (timeout % 1000) * 1000000;
  ts.tv_sec += timeout / 1000 + ts.tv_nsec / 1000000000;
  ts.tv_nsec %= 1000000000;
  return sem_timedwait((sem_t*)&handle, &ts) != -1;
#endif
}

bool Semaphore::tryWait()
{
#ifdef _WIN32
  return WaitForSingleObject((HANDLE)handle, 0) == WAIT_OBJECT_0;
#else
  return sem_trywait((sem_t*)&handle) != -1;
#endif
}
