
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
  // todo
#endif
}

Signal::~Signal()
{
#ifdef _WIN32
  VERIFY(CloseHandle(handle));
#else
  // todo
#endif
}

void_t Signal::set()
{
#ifdef _WIN32
  VERIFY(SetEvent(handle));
#else
  // todo
#endif
}

void_t Signal::reset()
{
#ifdef _WIN32
  VERIFY(ResetEvent(handle));
#else
  // todo
#endif
}

bool_t Signal::wait()
{
#ifdef _WIN32
  return WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0;
#else
  // todo
  return false;
#endif
}
