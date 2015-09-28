
#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#include <unistd.h> // usleep
#endif

#include <nstd/Debug.h>
#include <nstd/Thread.h>

Thread::Thread() : thread(0)//, threadId(0)
{
#ifdef _WIN32
  ASSERT(sizeof(thread) >= sizeof(HANDLE));
#else
  ASSERT(sizeof(thread) >= sizeof(pthread_t));
#endif
}

Thread::~Thread()
{
  if(thread)
    join();
}

bool_t Thread::start(uint_t (*proc)(void_t*), void_t* param)
{
  if(thread)
    return false;
#ifdef _WIN32
  //DWORD threadId;
  ASSERT(sizeof(unsigned long) == sizeof(uint_t));

#ifndef __CYGWIN__
  thread = (void_t*)CreateThread(0, 0, (unsigned long (__stdcall*)(void*)) proc, param, 0, /*&threadId*/0);
  if(!thread)
    return false;
#else
  struct ThreadData
  {
    static DWORD WINAPI ThreadProc(LPVOID lpParameter)
    {
      ThreadData& data = *(ThreadData*)lpParameter;
      uint_t (*proc)(void_t*) = data.proc;
      void_t* param = data.param;
      VERIFY(SetEvent(data.hevent));
      // return proc(param); // this does not set the exit code
      ExitThread(proc(param));
    }
    uint_t (*proc)(void_t*);
    void_t* param;
    HANDLE hevent;
  } data;
  
  data.proc = proc;
  data.param = param;
  data.hevent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if(data.hevent == NULL)
    return false;
  thread = (void_t*)CreateThread(0, 0, ThreadData::ThreadProc, &data, 0, /*&threadId*/0);
  if(!thread)
  {
    VERIFY(CloseHandle(data.hevent));
    return false;
  }
  VERIFY(WaitForSingleObject(data.hevent, INFINITE) == WAIT_OBJECT_0);
  VERIFY(CloseHandle(data.hevent));
#endif
  //this->threadId = threadId;
  return true;
#else
  pthread_t thread;
  if(pthread_create(&thread, 0, (void* (*) (void *)) proc, param) != 0)
    return false;
  this->thread = (void_t*)thread;
  return true;
#endif
}

uint_t Thread::join()
{
  if(!thread)
    return 0;
#ifdef _WIN32
  VERIFY(WaitForSingleObject(thread, INFINITE) == WAIT_OBJECT_0);
  DWORD exitCode;
  VERIFY(GetExitCodeThread(thread, &exitCode));
  VERIFY(CloseHandle(thread));
  thread = 0;
  //threadId = 0;
  return exitCode;
#else
  void* retval;
  VERIFY(pthread_join((pthread_t)thread, &retval) == 0);
  thread = 0;
  return (uint_t)(intptr_t)retval;
#endif
}

void_t Thread::yield()
{
#ifdef _WIN32
  SwitchToThread();
#else
  pthread_yield();
#endif
}

void_t Thread::sleep(int64_t milliseconds)
{
#ifdef _WIN32
  Sleep((DWORD)milliseconds);
#else
  usleep(milliseconds * 1000);
#endif
}

uint32_t Thread::getCurrentThreadId()
{
#ifdef _WIN32
  return (uint32_t)GetCurrentThreadId();
#else
  return (uint32_t)pthread_self();
#endif
}
