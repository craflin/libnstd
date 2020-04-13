
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <pthread.h>
#include <unistd.h> // usleep
#include <sys/syscall.h>
#endif

#include <nstd/Debug.hpp>
#include <nstd/Thread.hpp>

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

bool Thread::start(uint (*proc)(void*), void* param)
{
  if(thread)
    return false;
#ifdef _WIN32
  //DWORD threadId;
  ASSERT(sizeof(unsigned long) == sizeof(uint));

#ifndef __CYGWIN__
  thread = (void*)CreateThread(0, 0, (unsigned long (__stdcall*)(void*)) proc, param, 0, /*&threadId*/0);
  if(!thread)
    return false;
#else
  struct ThreadData
  {
    static DWORD WINAPI ThreadProc(LPVOID lpParameter)
    {
      ThreadData& data = *(ThreadData*)lpParameter;
      uint (*proc)(void*) = data.proc;
      void* param = data.param;
      VERIFY(SetEvent(data.hevent));
      // return proc(param); // this does not set the exit code
      ExitThread(proc(param));
    }
    uint (*proc)(void*);
    void* param;
    HANDLE hevent;
  } data;
  
  data.proc = proc;
  data.param = param;
  data.hevent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if(data.hevent == NULL)
    return false;
  thread = (void*)CreateThread(0, 0, ThreadData::ThreadProc, &data, 0, /*&threadId*/0);
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
  this->thread = (void*)thread;
  return true;
#endif
}

uint Thread::join()
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
  return (uint)(intptr_t)retval;
#endif
}

void Thread::yield()
{
#ifdef _WIN32
  SwitchToThread();
#else
  pthread_yield();
#endif
}

void Thread::sleep(int64 milliseconds)
{
#ifdef _WIN32
  Sleep((DWORD)milliseconds);
#else
  usleep(milliseconds * 1000);
#endif
}

uint32 Thread::getCurrentThreadId()
{
#ifdef _WIN32
  return (uint32)GetCurrentThreadId();
#else
  return (uint32)syscall(__NR_gettid);
#endif
}
