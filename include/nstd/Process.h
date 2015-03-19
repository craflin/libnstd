
#pragma once

#include <nstd/String.h>

class Process
{
public:

  Process();
  ~Process();

  /**
  * Start an external process.
  * @param  command The command to start the process. The first word in \c command should be the name of the executable or a path
  *                 to the executable. Further words in \c command are arguments for the process.
  * @return The process id of the newly started process or \c 0 if an errors occurred.
  */
  uint32_t start(const String& command);

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
  bool_t join(uint32_t& exitCode);

  /**
  * Kill and join the process. The method send a TERM signal to the process and waits for it to terminate.
  * @return Whether the process terminated properly.
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

  static Process* wait(Process* processes, size_t count);
  static void_t interrupt();

#ifndef _WIN32
  static bool_t daemonize(const String& logFile = "/dev/null");
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
    int_t character;
    const tchar_t* name;
    uint32_t flags;
  };

  class Arguments
  {
  public:
    template<size_t N> Arguments(int_t argc, tchar_t* argv[], const Option(&options)[N]) : argv(argv), argvEnd(argv + argc), options(options), optionsEnd(options + N), arg(_T("")), inOpt(false), skipOpt(false) {++this->argv;}

    bool_t read(int_t& character, String& argument);

  private:
    tchar_t** argv;
    tchar_t** argvEnd;
    const Option* options;
    const Option* optionsEnd;
    const tchar_t* arg;
    bool_t inOpt;
    bool_t skipOpt;

  private:
    bool_t nextChar();
  };

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
