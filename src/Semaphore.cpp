/**
* Implementation of a Semaphore object.
* @author Colin Graf
*/

#ifdef _WIN32
#include <climits>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <unistd.h> // usleep
#endif

#include <nstd/Semaphore.hpp>
#include <nstd/Debug.hpp>

Semaphore::Semaphore(uint value)
{
#ifdef _WIN32
  VERIFY(handle = CreateSemaphore(NULL, value, LONG_MAX, NULL));
#else
  ASSERT(sizeof(data) >= sizeof(sem_t));
  VERIFY(sem_init((sem_t*)data, 0, value) != -1);
#endif
}

Semaphore::~Semaphore()
{
#ifdef _WIN32
  VERIFY(CloseHandle(handle));
#else
  VERIFY(sem_destroy((sem_t*)data) != -1);
#endif
}

void Semaphore::signal()
{
#ifdef _WIN32
  VERIFY(ReleaseSemaphore((HANDLE)handle, 1, 0));
#else
  VERIFY(sem_post((sem_t*)data) != -1);
#endif
}

bool Semaphore::wait()
{
#ifdef _WIN32
  return WaitForSingleObject((HANDLE)handle, INFINITE) == WAIT_OBJECT_0;
#else
  return sem_wait((sem_t*)data) != -1;
#endif
}

bool Semaphore::wait(int64 timeout)
{
#ifdef _WIN32
  return WaitForSingleObject((HANDLE)handle, (DWORD)timeout) == WAIT_OBJECT_0;
#else
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_nsec += (timeout % 1000) * 1000000;
  ts.tv_sec += timeout / 1000 + ts.tv_nsec / 1000000000;
  ts.tv_nsec %= 1000000000;
  for(;;)
  {
    if(sem_timedwait((sem_t*)data, &ts) == -1)
    {
      if(errno == EINTR)
        continue;
      if(errno == ENOSYS)
        goto no_sem_timedwait;
      return false;
    }
    return true;
  }
no_sem_timedwait:
  // TODO: this sucks, find a better way to do something like this:
  for(int i = 0; i < timeout; i += 10)
  {
    if(sem_trywait((sem_t*)data) != -1)
      return true;
    usleep(10 * 1000);
  }
  return false;
#endif
}

bool Semaphore::tryWait()
{
#ifdef _WIN32
  return WaitForSingleObject((HANDLE)handle, 0) == WAIT_OBJECT_0;
#else
  return sem_trywait((sem_t*)data) != -1;
#endif
}

