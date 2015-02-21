
#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

#include <nstd/Signal.h>
#include <nstd/Debug.h>

Signal::Signal(bool set)
{
#ifdef _WIN32
  VERIFY(handle = CreateEvent(NULL, TRUE, set ? TRUE : FALSE, NULL));
#else
  ASSERT(sizeof(cdata) >= sizeof(pthread_cond_t));
  ASSERT(sizeof(mdata) >= sizeof(pthread_mutex_t));
#if __cplusplus >= 201103L
  *(pthread_cond_t*)cdata = PTHREAD_COND_INITIALIZER;
  *(pthread_mutex_t*)mdata = PTHREAD_MUTEX_INITIALIZER;
#else
  pthread_cond_init((pthread_cond_t*)cdata, 0);
  pthread_mutex_init((pthread_mutex_t*)mdata, 0);
#endif
  if(set)
  {
    signaled = true;
    VERIFY(pthread_cond_signal((pthread_cond_t*)cdata) == 0);
  }
  else
    signaled = false;
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

void_t Signal::set()
{
#ifdef _WIN32
  VERIFY(SetEvent(handle));
#else
  VERIFY(pthread_mutex_lock((pthread_mutex_t*)mdata) == 0);
  signaled = true;
  VERIFY(pthread_mutex_unlock((pthread_mutex_t*)mdata) == 0);
  VERIFY(pthread_cond_signal((pthread_cond_t*)cdata) == 0);
#endif
}

void_t Signal::reset()
{
#ifdef _WIN32
  VERIFY(ResetEvent(handle));
#else
  VERIFY(pthread_mutex_lock((pthread_mutex_t*)mdata) == 0);
  signaled = false;
  VERIFY(pthread_mutex_unlock((pthread_mutex_t*)mdata) == 0);
#endif
}

bool_t Signal::wait()
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

bool_t Signal::wait(timestamp_t timeout)
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
