
#pragma once

#include <nstd/String.h>

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

  /**
  * Kill and join the process.
  * @return Whether the proces was properly joined.
  */
  bool_t kill();

  enum Stream
  {
    stdoutStream = 0x01,
    stderrStream = 0x02,
    stdinStream = 0x04,
  };

  bool_t open(const String& command, uint_t streams = stdoutStream);

  ssize_t read(void_t* buffer, size_t length);
  ssize_t read(void_t* buffer, size_t length, uint_t& streams);
  ssize_t write(const void_t* buffer, size_t length);


  /*
  * Wait for one of the given processes to exit.
  */
  //static Process* joinOne(Process* processes, size_t count, uint32_t& exitCode);

  static uint32_t getCurrentProcessId();

  static void_t exit(uint32_t exitCode);

  //static void_t getEnvironmentVariables(HashMap<String, String>& vars);

  static String getEnvironmentVariable(const String& name);
  static bool_t setEnvironmentVariable(const String& name, const String& value);

private:
#ifdef _WIN32
  void_t* hProcess;
  void_t* hStdOutRead;
  void_t* hStdErrRead;
  void_t* hStdInWrite;
#else
  uint32_t pid;
  int_t fdStdOutRead;
  int_t fdStdErrRead;
  int_t fdStdInWrite;
#endif

  Process(const Process&);
  Process& operator=(const Process&);
};
