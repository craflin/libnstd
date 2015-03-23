
#include <nstd/Debug.h>
#include <nstd/Process.h>
#include <nstd/Thread.h>

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
         }
  /*
  struct InterrupterThread
  {
    static uint32_t proc(void*)
    {
      Thread::sleep(30);
      Process::interrupt();
    }
  };
  Thread thread;
  thread.start(proc, )
  */
}
