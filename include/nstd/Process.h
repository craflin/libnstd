
#pragma once

#include <nstd/String.h>
#include <nstd/HashMap.h>

class Process
{
public:

  Process();
  ~Process();

  /**
  * Start an external process.
  * @param  command The command to start the proces. The first word in \c command should be the name of the executable or a path 
  *                 to the executable. Further words in \c command are used as arguments the process.
  * @return The process id of the newly started process or \c 0 if an errors occured
  */
  uint32_t start(const String& command);

  /**
  * Return the running state of the process.
  * @return \c true when the process is currently running and can be joined using \c join()
  */
  bool isRunning() const;

  /**
  * Wait for the process to terminate and get its exit code.
  * @param[out] exitCode The exit code of the process.
  * @return Whether the process terminated properly.
  */
  bool_t join(uint32_t& exitCode);

  /*
  * Wait for one of the given processes to exit.
  */
  //static Process* joinOne(Process* processes, size_t count, uint32_t& exitCode);

  //static void_t getEnvironmentVariables(HashMap<String, String>& vars);

private:
#ifdef _WIN32
  void* hProcess;
#else
  uint32_t pid;
#endif
};
