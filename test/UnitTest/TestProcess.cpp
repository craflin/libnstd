
#include <nstd/Process.hpp>
#include <nstd/Debug.hpp>
#include <nstd/Thread.hpp>
#include <nstd/Time.hpp>

void testProcess()
{
  // test start() and join()
  {
    Process process;
#ifdef _WIN32
    uint32 id = process.start("cmd /C \"choice /T 1 /D N >NUL\"");
#else
    uint32 id = process.start("sleep 1");
#endif
    ASSERT(id != 0);
    ASSERT(process.isRunning());
    uint32 exitCode = 0xfffa2;
    ASSERT(process.join(exitCode));
    ASSERT(!process.isRunning());
#ifdef _WIN32
    ASSERT(exitCode == 2 || exitCode == 255);
#else
    ASSERT(exitCode == 0);
#endif
  }

  // test open(), read() and join()
  {
    Process process;
#ifdef _WIN32
    ASSERT(process.open("cmd /C echo 123", Process::stdoutStream));
#else
    ASSERT(process.open("ls", Process::stdoutStream));
#endif
    ASSERT(process.isRunning());
    char buffer[123];
    ASSERT(process.read(buffer, sizeof(buffer)) > 0);
    uint32 exitCode = 0xfffa2;
    ASSERT(process.join(exitCode));
    ASSERT(!process.isRunning());
    ASSERT(exitCode == 0);
  }

  // test wait and interrupt
  {
    Process::interrupt();
    Process* processes[1];
    int64 start = Time::ticks();
    ASSERT(Process::wait(processes, 0) == 0);
    int64 waitDuration = Time::ticks() - start;
    ASSERT(waitDuration < 10);
  }
  {
    struct InterrupterThread
    {
      static uint32 proc(void*)
      {
        Thread::sleep(50);
        Process::interrupt();
        return 0;
      }
    };
    Thread thread;
    thread.start(InterrupterThread::proc, 0);
    Process* processes[1];
    int64 start = Time::ticks();
    ASSERT(Process::wait(processes, 0) == 0);
    int64 waitDuration = Time::ticks() - start;
    ASSERT(waitDuration > 20);
  }
}

void testWait()
{
  Process process;
  Process process2;
#ifdef _WIN32
  ASSERT(process.open("cmd /C echo 123") != 0);
  ASSERT(process2.start("cmd /C \"choice /T 1 /D N >NUL\"") != 0);
#else
  ASSERT(process.open("ls") != 0);
  ASSERT(process2.start("sleep 1") != 0);
#endif
  Process* processes[] = {&process2, &process};
  ASSERT(Process::wait(processes, 2) == &process);
  uint32 exitCode;
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

void testStartEnv()
{
  Process process;
  Map<String, String> env;
  env.insert("ENV_TEST", "42");
#ifdef _WIN32
  process.start("cmd /c \"exit %ENV_TEST%\"", env);
#else
  process.start("sh -c \"exit $ENV_TEST\"", env);
#endif
  uint32 exitCode = 0;
  process.join(exitCode);
  ASSERT(exitCode == 42);
}

void testOpenEnv()
{
  Process process;
  Map<String, String> env;
  env.insert("ENV_TEST", "42");
#ifdef _WIN32
  process.start("cmd /c \"exit %ENV_TEST%\"", env);
#else
  process.start("sh -c \"exit $ENV_TEST\"", env);
#endif
  uint32 exitCode = 0;
  process.join(exitCode);
  ASSERT(exitCode == 42);
}

int main(int argc, char* argv[])
{
  testProcess();
  testWait();
  testStartEnv();
  testOpenEnv();
  return 0;
}
