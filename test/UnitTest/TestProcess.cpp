
#include <nstd/Debug.h>
#include <nstd/Process.h>
#include <nstd/Thread.h>
#include <nstd/Time.h>

void_t testProcess()
{
  // test start() and join()
  {
    Process process;
    uint32_t id = process.start(_T("sleep 1"));
    ASSERT(id != 0);
    ASSERT(process.isRunning());
    uint32_t exitCode = 0xfffa2;
    ASSERT(process.join(exitCode));
    ASSERT(!process.isRunning());
    ASSERT(exitCode == 0);
  }

  // test open(), read() and join()
  {
    Process process;
    ASSERT(process.open(_T("ls"), Process::stdoutStream));
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
    timestamp_t start = Time::ticks();
    ASSERT(Process::wait(processes, 0) == 0);
    timestamp_t waitDuration = Time::ticks() - start;
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
    timestamp_t start = Time::ticks();
    ASSERT(Process::wait(processes, 0) == 0);
    timestamp_t waitDuration = Time::ticks() - start;
    ASSERT(waitDuration > 20);
  }

  // test wait
  {
    Process process;
    Process process2;
    ASSERT(process.open(_T("ls")) != 0);
    ASSERT(process2.start(_T("sleep 1")) != 0);
    Process* processes[] = {&process2, &process};
    ASSERT(Process::wait(processes, 2) == &process);
    ASSERT(Process::wait(processes, 1) == &process2);
  }
}
