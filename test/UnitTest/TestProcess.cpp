
#include <nstd/Debug.h>
#include <nstd/Process.h>
#include <nstd/Thread.h>
#include <nstd/Time.h>

void_t testProcess()
{
  // test start() and join()
  {
    Process process;
#ifdef _WIN32
    uint32_t id = process.start(_T("cmd /C \"choice /T 1 /D N >NUL\""));
#else
    uint32_t id = process.start(_T("sleep 1"));
#endif
    ASSERT(id != 0);
    ASSERT(process.isRunning());
    uint32_t exitCode = 0xfffa2;
    ASSERT(process.join(exitCode));
    ASSERT(!process.isRunning());
#ifdef _WIN32
    ASSERT(exitCode == 2);
#else
    ASSERT(exitCode == 0);
#endif
  }

  // test open(), read() and join()
  {
    Process process;
#ifdef _WIN32
    ASSERT(process.open(_T("cmd /C dir"), Process::stdoutStream));
#else
    ASSERT(process.open(_T("ls"), Process::stdoutStream));
#endif
    ASSERT(process.isRunning());
    char_t buffer[123];
    ASSERT(process.read(buffer, sizeof(buffer)) > 0);
    uint32_t exitCode = 0xfffa2;
    ASSERT(process.join(exitCode));
    ASSERT(!process.isRunning());
    ASSERT(exitCode == 0);
  }

  // test wait and interrupt
  {
    Process::interrupt();
    Process* processes[1];
    int64_t start = Time::ticks();
    ASSERT(Process::wait(processes, 0) == 0);
    int64_t waitDuration = Time::ticks() - start;
    ASSERT(waitDuration < 10);
  }
  {
    struct InterrupterThread
    {
      static uint32_t proc(void*)
      {
        Thread::sleep(30);
        Process::interrupt();
        return 0;
      }
    };
    Thread thread;
    thread.start(InterrupterThread::proc, 0);
    Process* processes[1];
    int64_t start = Time::ticks();
    ASSERT(Process::wait(processes, 0) == 0);
    int64_t waitDuration = Time::ticks() - start;
    ASSERT(waitDuration > 20);
  }

  // test wait
  {
    Process process;
    Process process2;
#ifdef _WIN32
    ASSERT(process.open(_T("cmd /C dir")) != 0);
    ASSERT(process2.start(_T("cmd /C \"choice /T 1 /D N >NUL\"")) != 0);
#else
    ASSERT(process.open(_T("ls")) != 0);
    ASSERT(process2.start(_T("sleep 1")) != 0);
#endif
    Process* processes[] = {&process2, &process};
    ASSERT(Process::wait(processes, 2) == &process);
    uint32_t exitCode;
    ASSERT(process.join(exitCode));
    ASSERT(exitCode == 0);
    ASSERT(Process::wait(processes, 1) == &process2);
    ASSERT(process2.join(exitCode));
#ifdef _WIN32
    ASSERT(exitCode == 2);
#else
    ASSERT(exitCode == 0);
#endif
  }
}
