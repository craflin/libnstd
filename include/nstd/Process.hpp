
#pragma once

#include <nstd/String.hpp>

class Process
{
public:

  Process();
  ~Process();

  /**
  * Start an external process.
  * @param[in]  command The command to start the process. The first word in \c command should be the name of the executable or a path
  *                     to the executable. Further words in \c command are considered to be arguments for the process.
  * @return The process id of the newly started process or \c 0 if an errors occurred.
  */
  uint32 start(const String& command);

  /**
  * Start an external process.
  * @param[in]  executable  The path to the executable to be started.
  * @param[in]  argc        The amount of parameters in \c argv.
  * @param[in]  argv        Arguments to the process.
  * @return The process id of the newly started process or \c 0 if an errors occurred.
  */
  uint32 start(const String& executable, int argc, tchar* const argv[]);

  /**
  * Get id of the process.
  * @return The process id or \c 0 if a process was not started.
  */
  uint32 getProcessId() const {return pid;}

  /**
  * Return the running state of the process.
  * @return \c true when the process is currently running and can be joined using \c join().
  */
  bool isRunning() const;

  /**
  * Wait for the process to terminate and get its exit code.
  * @param[out] exitCode The exit code of the process.
  * @return Whether the process terminated properly.
  */
  bool join(uint32& exitCode);

  /**
  * Wait for the process to terminate.
  * @return Whether the process terminated properly.
  */
  bool join();

  /**
  * Kill and join the process. The method send a hard KILL signal to the process and waits for it to terminate.
  * @return Whether the process terminated properly.
  */
  bool kill();

  enum Stream
  {
    stdoutStream = 0x01,
    stderrStream = 0x02,
    stdinStream = 0x04,
  };

  bool open(const String& command, uint streams = stdoutStream);

  bool open(const String& executable, int argc, tchar* const argv[], uint streams = stdoutStream);

  bool open(const String& executable, const List<String>& args, uint streams = stdoutStream);

  /*
  * Closes the streams of an opened process.
  * @param [in] streams The streams to be closed.
  */
  void close(uint streams = stdoutStream | stderrStream | stdinStream);

  ssize read(void* buffer, usize length);
  ssize read(void* buffer, usize length, uint& streams);
  ssize write(const void* buffer, usize length);

  static uint32 getCurrentProcessId();

  static void exit(uint32 exitCode);

  static String getEnvironmentVariable(const String& name, const String& defaultValue = String());
  static bool setEnvironmentVariable(const String& name, const String& value);
  static String getExecutablePath();

  static Process* wait(Process** processes, usize count);
  static void interrupt();

#ifndef _WIN32
  static bool daemonize(const String& logFile = "/dev/null");
#endif

public:
  enum OptionFlags
  {
    optionFlag = 0x0,
    argumentFlag = 0x1,
    optionalFlag = 0x2,
  };

  struct Option
  {
    int character;
    const tchar* name;
    uint32 flags;
  };

  class Arguments
  {
  public:
    template<usize N> Arguments(int argc, tchar* argv[], const Option(&options)[N]) : argv(argv), argvEnd(argv + argc), options(options), optionsEnd(options + N), arg(_T("")), inOpt(false), skipOpt(false) {++this->argv;}

    bool read(int& character, String& argument);

  private:
    tchar** argv;
    tchar** argvEnd;
    const Option* options;
    const Option* optionsEnd;
    const tchar* arg;
    bool inOpt;
    bool skipOpt;

  private:
    bool nextChar();
  };

private:
#ifdef _WIN32
  void* hProcess;
  void* hStdOutRead;
  void* hStdErrRead;
  void* hStdInWrite;
#else
  int fdStdOutRead;
  int fdStdErrRead;
  int fdStdInWrite;
#endif
  uint32 pid;

  Process(const Process&);
  Process& operator=(const Process&);

private:
  class Private;
};
