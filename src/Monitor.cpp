
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <pthread.h>
#endif

#include <nstd/Monitor.hpp>
#include <nstd/Debug.hpp>

Monitor::Monitor() : signaled(false)
{
#ifdef _WIN32
  ASSERT(sizeof(cdata) >= sizeof(CONDITION_VARIABLE));
  ASSERT(sizeof(mdata) >= sizeof(CRITICAL_SECTION));
  (CONDITION_VARIABLE&)cdata = CONDITION_VARIABLE_INIT;
  InitializeCriticalSection((CRITICAL_SECTION*)mdata);
#else
  ASSERT(sizeof(cdata) >= sizeof(pthread_cond_t));
  ASSERT(sizeof(mdata) >= sizeof(pthread_mutex_t));
  pthread_cond_init((pthread_cond_t*)cdata, 0);
  pthread_mutex_init((pthread_mutex_t*)mdata, 0);
#endif
}

Monitor::~Monitor()
{
#ifdef _WIN32
  DeleteCriticalSection((CRITICAL_SECTION*)mdata);
#else
  VERIFY(pthread_cond_destroy((pthread_cond_t*)cdata) == 0);
  VERIFY(pthread_mutex_destroy((pthread_mutex_t*)mdata) == 0);
#endif
}

bool Monitor::tryLock()
{
#ifdef _WIN32
  return TryEnterCriticalSection((CRITICAL_SECTION*)mdata) == TRUE;
#else
  return pthread_mutex_trylock((pthread_mutex_t*)mdata) == 0;
#endif
}

void Monitor::lock()
{
#ifdef _WIN32
  EnterCriticalSection((CRITICAL_SECTION*)mdata);
#else
  VERIFY(pthread_mutex_lock((pthread_mutex_t*)mdata) == 0);
#endif
}

bool Monitor::wait()
{
#ifdef _WIN32
  for(;;)
  {
    if(!SleepConditionVariableCS((CONDITION_VARIABLE*)cdata, (CRITICAL_SECTION*)mdata, INFINITE))
      return false;
    if(signaled)
    {
      signaled = false;
      return true;
    }
  }
#else
  for(;;)
  {
    VERIFY(pthread_cond_wait((pthread_cond_t*)cdata, (pthread_mutex_t*)mdata) == 0);
    if(signaled)
    {
      signaled = false;
      return true;
    }
  }
#endif
}

bool Monitor::wait(int64 timeout)
{
#ifdef _WIN32
  for(;;)
  {
    if(!SleepConditionVariableCS((CONDITION_VARIABLE*)cdata, (CRITICAL_SECTION*)mdata, (DWORD)timeout))
      return false;
    if(signaled)
    {
      signaled = false;
      return true;
    }
  }
#else
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_nsec += (timeout % 1000) * 1000000;
  ts.tv_sec += timeout / 1000 + ts.tv_nsec / 1000000000;
  ts.tv_nsec %= 1000000000;
  for(;;)
  {
    if(pthread_cond_timedwait((pthread_cond_t*)cdata, (pthread_mutex_t*)mdata, &ts) != 0)
      return false;
    if(signaled)
    {
      signaled = false;
      return true;
    }
  }
#endif
}

void Monitor::unlock()
{
#ifdef _WIN32
  LeaveCriticalSection((CRITICAL_SECTION*)mdata);
#else
  VERIFY(pthread_mutex_unlock((pthread_mutex_t*)mdata) == 0);
#endif
}

void Monitor::set()
{
#ifdef _WIN32
  EnterCriticalSection((CRITICAL_SECTION*)mdata);
#else
  VERIFY(pthread_mutex_lock((pthread_mutex_t*)mdata) == 0);
#endif
  signaled = true;
#ifdef _WIN32
  LeaveCriticalSection((CRITICAL_SECTION*)mdata);
  WakeConditionVariable((CONDITION_VARIABLE*)cdata);
#else
  VERIFY(pthread_mutex_unlock((pthread_mutex_t*)mdata) == 0);
  VERIFY(pthread_cond_signal((pthread_cond_t*)cdata) == 0);
#endif
}
