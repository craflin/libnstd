
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <pthread.h>
#endif

#include <nstd/Signal.hpp>
#include <nstd/Debug.hpp>

Signal::Signal(bool set)
{
#ifdef _WIN32
  VERIFY(handle = CreateEvent(NULL, TRUE, set ? TRUE : FALSE, NULL));
#else
  ASSERT(sizeof(cdata) >= sizeof(pthread_cond_t));
  ASSERT(sizeof(mdata) >= sizeof(pthread_mutex_t));
  pthread_cond_init((pthread_cond_t*)cdata, 0);
  pthread_mutex_init((pthread_mutex_t*)mdata, 0);
  signaled = set;
#endif
}

Signal::~Signal()
{
#ifdef _WIN32
  VERIFY(CloseHandle(handle));
#else
  VERIFY(pthread_cond_destroy((pthread_cond_t*)cdata) == 0);
  VERIFY(pthread_mutex_destroy((pthread_mutex_t*)mdata) == 0);
#endif
}

void Signal::set()
{
#ifdef _WIN32
  VERIFY(SetEvent(handle));
#else
  VERIFY(pthread_mutex_lock((pthread_mutex_t*)mdata) == 0);
  signaled = true;
  VERIFY(pthread_mutex_unlock((pthread_mutex_t*)mdata) == 0);
  VERIFY(pthread_cond_broadcast((pthread_cond_t*)cdata) == 0);
#endif
}

void Signal::reset()
{
#ifdef _WIN32
  VERIFY(ResetEvent(handle));
#else
  VERIFY(pthread_mutex_lock((pthread_mutex_t*)mdata) == 0);
  signaled = false;
  VERIFY(pthread_mutex_unlock((pthread_mutex_t*)mdata) == 0);
#endif
}

bool Signal::wait()
{
#ifdef _WIN32
  return WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0;
#else
  VERIFY(pthread_mutex_lock((pthread_mutex_t*)mdata) == 0);
  for(;;)
  {
    if(signaled)
    {
      VERIFY(pthread_mutex_unlock((pthread_mutex_t*)mdata) == 0);
      return true;
    }
    VERIFY(pthread_cond_wait((pthread_cond_t*)cdata, (pthread_mutex_t*)mdata) == 0);
  }
#endif
}

bool Signal::wait(int64 timeout)
{
#ifdef _WIN32
  return WaitForSingleObject(handle, (DWORD)timeout) == WAIT_OBJECT_0;
#else
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_nsec += (timeout % 1000) * 1000000;
  ts.tv_sec += timeout / 1000 + ts.tv_nsec / 1000000000;
  ts.tv_nsec %= 1000000000;
  VERIFY(pthread_mutex_lock((pthread_mutex_t*)mdata) == 0);
  for(;;)
  {
    if(signaled)
    {
      VERIFY(pthread_mutex_unlock((pthread_mutex_t*)mdata) == 0);
      return true;
    }
    if(pthread_cond_timedwait((pthread_cond_t*)cdata, (pthread_mutex_t*)mdata, &ts) != 0)
    {
      VERIFY(pthread_mutex_unlock((pthread_mutex_t*)mdata) == 0);
      return false;
    }
  }
#endif
}
