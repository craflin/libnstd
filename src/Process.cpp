
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdlib>
#else
#include <alloca.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdio>
#include <cstring>
#include <signal.h>
#include <cstdlib> // setenv
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#endif

#include <nstd/Debug.hpp>
#include <nstd/List.hpp>
#ifndef _WIN32
#include <nstd/File.hpp>
#endif
#include <nstd/Process.hpp>
#include <nstd/Buffer.hpp>
#include <nstd/Array.hpp>

class Process::Private
{
public:
#ifdef _WIN32
  static String getCommandLine(const String& program, int argc, tchar* const argv[])
  {
    String commandLine = program;
    for (int i = 1; i < argc; ++i)
    {
      commandLine.append(' ');
      const tchar* arg = argv[i];
      if (!String::findOneOf(arg, _T(" \t\n\v\"")) && *arg)
        commandLine.append(arg, String::length(arg));
      else
      {
        commandLine.append('"');
        while (*arg)
        {
          if (*arg == '\\')
          {
            for (const tchar* p = arg + 1;; ++p)
              if (*p != '\\')
              {
                usize backslashCount = p - arg;
                if (!*p)
                {
                  commandLine.append(arg, backslashCount);
                  commandLine.append(arg, backslashCount);
                  arg = p;
                }
                else if (*p == '"')
                {
                  commandLine.append(arg, backslashCount);
                  commandLine.append(arg, backslashCount);
                  commandLine.append(_T("\\\""));
                  arg = p + 1;
                }
                else
                {
                  commandLine.append(arg, backslashCount);
                  arg = p;
                }
                break;
              }
          }
          else
          {
            commandLine.append(*arg);
            ++arg;
          }
        }
        commandLine.append('"');
      }
    }
    return commandLine;
  }
#else
  static void splitCommandLine(const String& commandLine, List<String>& command)
  {
    String arg;
    for (const tchar* p = commandLine; *p;)
      switch (*p)
      {
      case _T('"'):
        for (++p; *p;)
        {
          switch (*p)
          {
          case _T('"'):
            ++p;
            break;
          case _T('\\'):
            if (p[1] == _T('"'))
            {
              arg.append(_T('"'));
              p += 2;
            }
            continue;
          default:
            arg.append(*(p++));
            continue;
          }
          break;
        }
        break;
      case _T(' '):
        command.append(arg);
        arg.clear();
        ++p;
        break;
      default:
        arg.append(*(p++));
      }
    if (!arg.isEmpty())
      command.append(arg);
  }
#endif
};

Process::Process() : pid(0)
{
#ifdef _WIN32
  ASSERT(sizeof(hProcess) >= sizeof(HANDLE));
  hProcess = INVALID_HANDLE_VALUE;
  hStdOutRead = INVALID_HANDLE_VALUE;
  hStdErrRead = INVALID_HANDLE_VALUE;
  hStdInWrite = INVALID_HANDLE_VALUE;
#else
  fdStdOutRead = 0;
  fdStdErrRead = 0;
  fdStdInWrite = 0;
#endif
}

Process::~Process()
{
  uint32 exitCode;
  join(exitCode);
}

bool Process::isRunning() const
{
#ifdef _WIN32
  return hProcess != INVALID_HANDLE_VALUE;
#else
  return pid != 0;
#endif
}

uint32 Process::start(const String& commandLine)
{
#ifdef _WIN32
  if(hProcess != INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return 0;
  }

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  String args(commandLine);

  if(!CreateProcess(NULL, (tchar*)args, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    return 0;

  CloseHandle(pi.hThread);

  ASSERT(pi.hProcess);
  hProcess = pi.hProcess;
  pid = pi.dwProcessId;

  return pi.dwProcessId;

#else
  if(pid)
  {
    errno = EINVAL;
    return false;
  }

  List<String> command;
  Private::splitCommandLine(commandLine, command);
  if(command.isEmpty())
    command.append(String());

  char** argv = (char**)alloca(sizeof(char*) * (command.size() + 1));
  int i = 0;
  for (List<String>::Iterator j = command.begin(), end = command.end(); j != end; ++j)
    argv[i++] = (char*)(const char*)*j;
  argv[i] = 0;
  return start(command.front(), i + 1, argv);
#endif
}

uint32 Process::start(const String& program, int argc, tchar* const argv[])
{
#ifdef _WIN32
  return start(Private::getCommandLine(program, argc, argv));
#else
  if (pid)
  {
    errno = EINVAL;
    return false;
  }

  // prepare argv of child
  const char** args;
  if(argc && !argv[argc - 1])
    args = (const char**)argv;
  else
  {
    if(!argc)
      ++argc;
    args = (const char**)alloca(sizeof(const char*) * (argc + 1));
    args[argc] = 0;
    for (int i = 1; i < argc; ++i)
      args[i] = argv[i];
    args[0] = program;
  }

  // start process
  int r = vfork();
  if (r == -1)
    return 0;
  else if (r != 0) // parent
  {
    pid = r;
    return r;
  }
  else // child
  {
    if(execvp(program, (char* const*)args) == -1)
    {
      fprintf(stderr, "%s: %s\n", (const char*)program, strerror(errno));
      _exit(EXIT_FAILURE);
    }
    ASSERT(false); // unreachable
    return 0;
  }
#endif
}

bool Process::kill()
{
#ifdef _WIN32
  if(hProcess == INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return false;
  }
  TerminateProcess(hProcess, EXIT_FAILURE);
  if(WaitForSingleObject(hProcess, INFINITE) != WAIT_OBJECT_0)
    return false;
  CloseHandle((HANDLE)hProcess);
  hProcess = INVALID_HANDLE_VALUE;
  if(hStdOutRead != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hStdOutRead);
    hStdOutRead = INVALID_HANDLE_VALUE;
  }
  if(hStdErrRead != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hStdErrRead);
    hStdErrRead = INVALID_HANDLE_VALUE;
  }
  if(hStdInWrite != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hStdInWrite);
    hStdInWrite = INVALID_HANDLE_VALUE;
  }
  pid = 0;
  return true;
#else
  if(!pid)
  {
    errno = EINVAL;
    return false;
  }
  ::kill((pid_t)pid, SIGKILL);
  int status;
  if(waitpid(pid, &status, 0) != (pid_t)pid)
    return false;
  if(fdStdOutRead)
  {
    ::close(fdStdOutRead);
    fdStdOutRead = 0;
  }
  if(fdStdErrRead)
  {
    ::close(fdStdErrRead);
    fdStdErrRead = 0;
  }
  if(fdStdInWrite)
  {
    ::close(fdStdInWrite);
    fdStdInWrite = 0;
  }
  pid = 0;
  return true;
#endif
}

bool Process::join(uint32& exitCode)
{
#ifdef _WIN32
  if(hProcess == INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return false;
  }
  DWORD dwExitCode = 0;
  if(!GetExitCodeProcess(hProcess, &dwExitCode))
    return false;
  if(dwExitCode == STILL_ACTIVE)
  {
    if(WaitForSingleObject(hProcess, INFINITE) != WAIT_OBJECT_0)
      return false;
    if(!GetExitCodeProcess(hProcess, &dwExitCode))
      return false;
  }
  CloseHandle((HANDLE)hProcess);
  hProcess = INVALID_HANDLE_VALUE;
  exitCode = (uint32)dwExitCode;
  if(hStdOutRead != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hStdOutRead);
    hStdOutRead = INVALID_HANDLE_VALUE;
  }
  if(hStdErrRead != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hStdErrRead);
    hStdErrRead = INVALID_HANDLE_VALUE;
  }
  if(hStdInWrite != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hStdInWrite);
    hStdInWrite = INVALID_HANDLE_VALUE;
  }
  pid = 0;
  return true;
#else
  if(!pid)
  {
    errno = EINVAL;
    return false;
  }
  int status;
  if(waitpid(pid, &status, 0) != (pid_t)pid)
    return false;
  exitCode = WEXITSTATUS(status);
  if(fdStdOutRead)
  {
    ::close(fdStdOutRead);
    fdStdOutRead = 0;
  }
  if(fdStdErrRead)
  {
    ::close(fdStdErrRead);
    fdStdErrRead = 0;
  }
  if(fdStdInWrite)
  {
    ::close(fdStdInWrite);
    fdStdInWrite = 0;
  }
  pid = 0;
  return true;
#endif
}

bool Process::join()
{
  uint32 exitCode;
  return join(exitCode);
}

uint32 Process::getCurrentProcessId()
{
#ifdef _WIN32
  return (uint32)GetCurrentProcessId();
#else
  return (uint32)getpid();
#endif
}

void Process::exit(uint32 exitCode)
{
#ifdef _WIN32
  ExitProcess(exitCode);
#else
  _exit(0);
#endif
}

bool Process::open(const String& commandLine, uint streams)
{
#ifdef _WIN32
  if(hProcess != INVALID_HANDLE_VALUE)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return false;
  }

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdInput = INVALID_HANDLE_VALUE;
  si.hStdOutput = INVALID_HANDLE_VALUE;
  si.hStdError = INVALID_HANDLE_VALUE;

  SECURITY_ATTRIBUTES sa; 
  sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
  sa.bInheritHandle = TRUE; 
  sa.lpSecurityDescriptor = NULL; 

  if(streams & stdoutStream)
  {
   if(!CreatePipe(&hStdOutRead, &si.hStdOutput, &sa, 0) ||
      !SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0))
      goto error;
  }
  else
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  if(streams & stderrStream)
  {
   if(!CreatePipe(&hStdErrRead, &si.hStdError, &sa, 0) ||
      !SetHandleInformation(hStdErrRead, HANDLE_FLAG_INHERIT, 0))
      goto error;
  }
  else
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
  if(streams & stdinStream)
  {
   if(!CreatePipe(&si.hStdInput, &hStdInWrite, &sa, 0) ||
      !SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0))
      goto error;
  }
  else
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

  {
    String args(commandLine);

    if(!CreateProcess(NULL, (tchar*)args, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
      goto error;
  }

  CloseHandle(pi.hThread);

  if(streams & stdoutStream && si.hStdOutput != INVALID_HANDLE_VALUE)
    CloseHandle(si.hStdOutput);
  if(streams & stderrStream && si.hStdError != INVALID_HANDLE_VALUE)
    CloseHandle(si.hStdError);
  if(streams & stdinStream && si.hStdInput != INVALID_HANDLE_VALUE)
    CloseHandle(si.hStdInput);

  ASSERT(pi.hProcess);
  hProcess = pi.hProcess;
  pid = pi.dwProcessId;
  return true;
error:
  DWORD err = GetLastError();
  if(hStdOutRead != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hStdOutRead);
    hStdOutRead = INVALID_HANDLE_VALUE;
  }
  if(hStdErrRead != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hStdErrRead);
    hStdErrRead = INVALID_HANDLE_VALUE;
  }
  if(hStdInWrite != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hStdInWrite);
    hStdInWrite = INVALID_HANDLE_VALUE;
  }
  if(streams & stdoutStream && si.hStdOutput != INVALID_HANDLE_VALUE)
    CloseHandle(si.hStdOutput);
  if(streams & stderrStream && si.hStdError != INVALID_HANDLE_VALUE)
    CloseHandle(si.hStdError);
  if(streams & stdinStream && si.hStdInput != INVALID_HANDLE_VALUE)
    CloseHandle(si.hStdInput);
  SetLastError(err);
  return false;
#else
  if(pid)
  {
    errno = EINVAL;
    return false;
  }

  List<String> command;
  Private::splitCommandLine(commandLine, command);
  if(command.isEmpty())
    command.append(String());

  char** argv = (char**)alloca(sizeof(char*) * (command.size() + 1));
  int i = 0;
  for (List<String>::Iterator j = command.begin(), end = command.end(); j != end; ++j)
    argv[i++] = (char*)(const char*)*j;
  argv[i] = 0;
  return open(command.front(), i + 1, argv, streams);
#endif
}

bool Process::open(const String& executable, int argc, tchar* const argv[], uint streams)
{
#ifdef _WIN32
  return open(Private::getCommandLine(executable, argc, argv), streams);
#else
  if (pid)
  {
    errno = EINVAL;
    return false;
  }

  // create pipes
  int stdoutFds[2] = {};
  int stderrFds[2] = {};
  int stdinFds[2] = {};
  if (streams & stdoutStream)
  {
    if (pipe(stdoutFds) != 0)
      goto error;
  }
  if (streams & stderrStream)
  {
    if (pipe(stderrFds) != 0)
      goto error;
  }
  if (streams & stdinStream)
  {
    if (pipe(stdinFds) != 0)
      goto error;
  }

  // prepare argv of child
  const char** args;
  if (argc && !argv[argc - 1])
    args = (const char**)argv;
  else
  {
    if (!argc)
      ++argc;
    args = (const char**)alloca(sizeof(const char*) * (argc + 1));
    args[argc] = 0;
    for (int i = 1; i < argc; ++i)
      args[i] = argv[i];
    args[0] = executable;
  }

  // start process
  {
    int r = vfork();
    if (r == -1)
      return false;
    else if (r != 0) // parent
    {
      pid = r;

      if (stdoutFds[1])
        ::close(stdoutFds[1]);
      if (stderrFds[1])
        ::close(stderrFds[1]);
      if (stdinFds[0])
        ::close(stdinFds[0]);

      fdStdOutRead = stdoutFds[0];
      fdStdErrRead = stderrFds[0];
      fdStdInWrite = stdinFds[1];

      return true;
    }
    else // child
    {
      if (stdoutFds[1])
      {
        dup2(stdoutFds[1], STDOUT_FILENO);
        ::close(stdoutFds[1]);
      }
      if (stderrFds[1])
      {
        dup2(stderrFds[1], STDERR_FILENO);
        ::close(stderrFds[1]);
      }
      if (stdinFds[0])
      {
        dup2(stdinFds[0], STDIN_FILENO);
        ::close(stdinFds[0]);
      }

      if (stdoutFds[0])
        ::close(stdoutFds[0]);
      if (stderrFds[0])
        ::close(stderrFds[0]);
      if (stdinFds[1])
        ::close(stdinFds[1]);

      if (execvp(executable, (char* const*)args) == -1)
      {
        fprintf(stderr, "%s: %s\n", (const char*)executable, strerror(errno));
        _exit(EXIT_FAILURE);
      }
      ASSERT(false); // unreachable
      return false;
    }
  }
error:
  int err = errno;
  if (stdoutFds[0])
  {
    ::close(stdoutFds[0]);
    ::close(stdoutFds[1]);
  }
  if (stderrFds[0])
  {
    ::close(stderrFds[0]);
    ::close(stderrFds[1]);
  }
  if (stdinFds[0])
  {
    ::close(stdinFds[0]);
    ::close(stdinFds[1]);
  }
  errno = err;
  return false;
#endif
}

bool Process::open(const String& executable, const List<String>& args, uint streams)
{
  Array<const tchar*> argv(args.size());
  for (List<String>::Iterator i = args.begin(), end = args.end(); i != end; ++i)
    argv.append((const tchar*)*i);
  return open(executable, (int)args.size(), (tchar**)(const tchar**)argv, streams);
}

void Process::close(uint streams)
{
#ifdef _WIN32
  if(streams & stdinStream && hStdInWrite != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hStdInWrite);
    hStdInWrite = INVALID_HANDLE_VALUE;
  }
  if(streams & stdoutStream && hStdOutRead != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hStdOutRead);
    hStdOutRead = INVALID_HANDLE_VALUE;
  }
  if(streams & stderrStream && hStdErrRead != INVALID_HANDLE_VALUE)
  {
    CloseHandle(hStdErrRead);
    hStdErrRead = INVALID_HANDLE_VALUE;
  }
#else
  if(streams & stdinStream && fdStdInWrite)
  {
    ::close(fdStdInWrite);
    fdStdInWrite = 0;
  }
  if(streams & stdoutStream && fdStdOutRead)
  {
    ::close(fdStdOutRead);
    fdStdOutRead = 0;
  }
  if(streams & stderrStream && fdStdErrRead)
  {
    ::close(fdStdErrRead);
    fdStdErrRead = 0;
  }
#endif
}

ssize Process::read(void* buffer, usize len)
{
#ifdef _WIN32
  DWORD i;
#ifdef _AMD64
  byte* bufferStart = (byte*)buffer;
  while(len > (usize)INT_MAX)
  {
    if(!ReadFile(hStdOutRead, buffer, INT_MAX, &i, NULL))
    {
      if(GetLastError() == ERROR_BROKEN_PIPE)
        return (byte*)buffer - bufferStart;
      return -1;
    }
    buffer = (byte*)buffer + i;
    if(i != INT_MAX)
      return (byte*)buffer - bufferStart;
    len -= INT_MAX;
  }
  if(!ReadFile(hStdOutRead, buffer, (DWORD)len, &i, NULL))
  {
    if(GetLastError() == ERROR_BROKEN_PIPE)
      return (byte*)buffer - bufferStart;
    return -1;
  }
  buffer = (byte*)buffer + i;
  return (byte*)buffer - bufferStart;
#else
  if(!ReadFile(hStdOutRead, buffer, len, &i, NULL))
  {
    if(GetLastError() == ERROR_BROKEN_PIPE)
      return 0;
    return -1;
  }
  return i;
#endif
#else
  return ::read(fdStdOutRead, buffer, len);
#endif
}

ssize Process::read(void* buffer, usize length, uint& streams)
{
#ifdef _WIN32
  HANDLE handles[2];
  DWORD handleCount = 0;
  if(streams & stdoutStream && hStdOutRead != INVALID_HANDLE_VALUE)
    handles[handleCount++] = hStdOutRead;
  if(streams & stderrStream && hStdErrRead != INVALID_HANDLE_VALUE)
    handles[handleCount++] = hStdErrRead;
  if(handleCount == 0)
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return -1;
  }
  DWORD dw = WaitForMultipleObjects(handleCount, handles, FALSE, INFINITE);
  if(dw < WAIT_OBJECT_0 || dw >= WAIT_OBJECT_0 + handleCount)
    return -1;
  HANDLE readHandle = handles[dw - WAIT_OBJECT_0];
  streams = readHandle == hStdOutRead ? stdoutStream : stderrStream;
  DWORD i;
#ifdef _AMD64
  byte* bufferStart = (byte*)buffer;
  while(length > (usize)INT_MAX)
  {
    if(!ReadFile(readHandle, buffer, INT_MAX, &i, NULL))
    {
      if(GetLastError() == ERROR_BROKEN_PIPE)
        return (byte*)buffer - bufferStart;
      return -1;
    }
    buffer = (byte*)buffer + i;
    if(i != INT_MAX)
      return (byte*)buffer - bufferStart;
    length -= INT_MAX;
  }
  if(!ReadFile(readHandle, buffer, (DWORD)length, &i, NULL))
  {
    if(GetLastError() == ERROR_BROKEN_PIPE)
      return (byte*)buffer - bufferStart;
    return -1;
  }
  buffer = (byte*)buffer + i;
  return (byte*)buffer - bufferStart;
#else
  if(!ReadFile(readHandle, buffer, length, &i, NULL))
    return -1;
  return i;
#endif
#else
  fd_set fdr;
  FD_ZERO(&fdr);
  int maxFd = 0;
  if(streams & stdoutStream && fdStdOutRead)
  {
    FD_SET(fdStdOutRead, &fdr);
    maxFd = fdStdOutRead;
  }
  if(streams & stderrStream && fdStdErrRead)
  {
    FD_SET(fdStdErrRead, &fdr);
    if(fdStdErrRead > maxFd)
      maxFd = fdStdErrRead;
  }
  if(maxFd == 0)
  {
    errno = EINVAL;
    return -1;
  }
  timeval tv = {1000, 0};
  for(;;)
  {
    int i;
    if((i = select(maxFd + 1, &fdr, 0, 0, &tv)) != 0)
      return -1;
    if(i == 0 || (i < 0 && errno == EINTR))
      continue;
  }
  if(streams & stdoutStream && fdStdOutRead && FD_ISSET(fdStdOutRead, &fdr))
  {
    streams = stdoutStream;
    return ::read(fdStdOutRead, buffer, length);
  }
  if(streams & stderrStream && fdStdErrRead && FD_ISSET(fdStdErrRead, &fdr))
  {
    streams = stderrStream;
    return ::read(fdStdErrRead, buffer, length);
  }
  return -1;
#endif
}

ssize Process::write(const void* buffer, usize len)
{
#ifdef _WIN32
  DWORD i;
#ifdef _AMD64
  const byte* bufferStart = (const byte*)buffer;
  while(len > (usize)INT_MAX)
  {
    if(!WriteFile(hStdInWrite, buffer, INT_MAX, &i, NULL))
      return -1;
    buffer = (const byte*)buffer + i;
    if(i != INT_MAX)
      return (const byte*)buffer - bufferStart;
    len -= INT_MAX;
  }
  if(!WriteFile(hStdInWrite, buffer, (DWORD)len, &i, NULL))
    return -1;
  buffer = (const byte*)buffer + i;
  return (const byte*)buffer - bufferStart;
#else
  if(!WriteFile(hStdInWrite, buffer, len, &i, NULL))
    return -1;
  return i;
#endif
#else
  return ::write(fdStdInWrite, buffer, len);
#endif
}

String Process::getEnvironmentVariable(const String& name, const String& defaultValue)
{
#ifdef _MSC_VER
  String buffer;
  DWORD bufferSize = 256;
  for(;;)
  {
    buffer.resize(bufferSize);
    DWORD dw = GetEnvironmentVariable((const tchar*)name, (tchar*)buffer, bufferSize);
    if(dw >= bufferSize)
    {
      bufferSize <<= 1;
      continue;
    }
    if(!dw)
    {
      if (GetLastError() ==  ERROR_ENVVAR_NOT_FOUND)
        return defaultValue;
      return String();
    }
    buffer.resize(dw);
    return buffer;
  }
#else
  const tchar* var = getenv((const tchar*)name);
  if(!var)
    return defaultValue;
  return String(var, String::length(var));
#endif
}

bool Process::setEnvironmentVariable(const String& name, const String& value)
{
#ifdef _MSC_VER
  return SetEnvironmentVariable((const tchar*)name, value.isEmpty() ? 0 : (const tchar*)value) == TRUE;
#else
  if(value.isEmpty())
    return unsetenv((const tchar*)name);
  return setenv((const tchar*)name, (const tchar*)value, 1) == 0;
#endif
}

bool Process::Arguments::nextChar()
{
  if(*arg)
  {
    ++arg;
    return true;
  }
  if(argv < argvEnd)
  {
    arg = *(argv++);
    inOpt = false;
    return true;
  }
  return false;
}

bool Process::Arguments::read(int& character, String& argument)
{
  if(!nextChar())
    return false;

  if(!inOpt && !skipOpt)
  {
    if(*arg == '-')
    {
      if(arg[1] == '-')
      { // handel '--' arg
        arg += 2;
        if(!*arg)
        {
          skipOpt = true;
          if(!nextChar())
            return false;
        }
        else
        {
          const tchar* end = String::find(arg, _T('='));
          usize argLen = end ? end - arg : String::length(arg);
          for(const Option* opt = options; opt < optionsEnd; ++opt)
            if(opt->name && String::compare(opt->name, arg, argLen) == 0 && !opt->name[argLen])
            {
              const tchar* argName = arg;
              character = opt->character;
              arg += end ? argLen + 1 : argLen;
              if(opt->flags & Process::argumentFlag)
              {
                if(end || (!(opt->flags & Process::optionalFlag) && nextChar()))
                {
                  usize len = String::length(arg);
                  argument.attach(arg, len);
                  arg += len;
                  return true;
                }
                if(!(opt->flags & Process::optionalFlag))
                {
                  // missing argument
                  argument.attach(argName - 2, argLen + 2);
                  character = ':';
                  return true;
                }
              }
              argument.clear();
              return true;
            }

          // unknown option
          character = '?';
          argLen += String::length(arg + argLen);
          argument.attach(arg - 2, argLen + 2);
          arg += argLen;
          return true;
        }
      }
      else
      {
        if(!*(++arg))
        {
          character = 0;
          argument.attach(arg - 1, 1); // "-"
          return true;
        }
        inOpt = true;
      }
    }
  }

  // find option *str
  if(inOpt)
  {
    character = *(arg++);
    for(const Option* opt = options; opt < optionsEnd; ++opt)
      if(opt->character == character)
      {
        if(opt->flags & Process::argumentFlag && !(opt->flags & Process::optionalFlag))
        {
          if(!*arg)
          {
            if(!nextChar())
            { // missing argument
              argument.clear();
              argument.append('-');
              argument.append((char)character);
              character = ':';
              return true;
            }
          }
          usize len = String::length(arg);
          argument.attach(arg, len);
          arg += len;
          return true;
        }
        argument.clear();
        return true;
      }

    // unknown option
    argument.clear();
    argument.append('-');
    argument.append((char)character);
    character = '?';
    return true;
  }

  // non option argument
  character = '\0';
  usize len = String::length(arg);
  argument.attach(arg, len);
  arg += len;
  return true;
}

#ifndef _WIN32
bool Process::daemonize(const String& logFile)
{
  int fd = ::open(logFile, O_CREAT | O_WRONLY |  O_CLOEXEC, S_IRUSR | S_IWUSR);
  if(fd == -1)
    return false;
  VERIFY(dup2(fd, STDOUT_FILENO) != -1);
  VERIFY(dup2(fd, STDERR_FILENO) != -1);
  ::close(fd);

  pid_t childPid = fork();
  if(childPid == -1)
    return false;
  if(childPid != 0)
  {
    exit(0);
    return false;
  }
  VERIFY(setsid() != -1);
  return true;
}
#endif

#ifdef _WIN32
static class ProcessFramework
{
public:
  static HANDLE hInterruptEvent;
  static CRITICAL_SECTION criticalSection;
  ProcessFramework() {InitializeCriticalSection(&criticalSection);}
  ~ProcessFramework()
  {
    DeleteCriticalSection(&criticalSection);
    if(hInterruptEvent != INVALID_HANDLE_VALUE)
      CloseHandle(hInterruptEvent);
  }
} processFramework;
HANDLE ProcessFramework::hInterruptEvent = INVALID_HANDLE_VALUE;;
CRITICAL_SECTION ProcessFramework::criticalSection;
#else
static class ProcessFramework
{
public:
  static pthread_mutex_t mutex;
  static pthread_cond_t condition;
  static int signaled;
  static int waitState;
  ProcessFramework()
  {
    VERIFY(pthread_mutex_init(&mutex, 0) == 0);
    VERIFY(pthread_cond_init(&condition, 0) == 0);
  }
  ~ProcessFramework()
  {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);
  }
} processFramework;
pthread_mutex_t ProcessFramework::mutex;
pthread_cond_t ProcessFramework::condition;
int ProcessFramework::signaled = 0;
int ProcessFramework::waitState = 0;
#endif

Process* Process::wait(Process** processes, usize count)
{
#ifdef _WIN32
  EnterCriticalSection(&ProcessFramework::criticalSection);
  if(ProcessFramework::hInterruptEvent == INVALID_HANDLE_VALUE)
    ProcessFramework::hInterruptEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  LeaveCriticalSection(&ProcessFramework::criticalSection);
  HANDLE handles[MAXIMUM_WAIT_OBJECTS];
  Process* processMap[MAXIMUM_WAIT_OBJECTS];
  Process** p = processMap;
  handles[0] = ProcessFramework::hInterruptEvent;
  Process** pend = processes + count;
  HANDLE* hend = handles + MAXIMUM_WAIT_OBJECTS;
  HANDLE* h = handles + 1;
  for(; processes < pend; ++processes)
  {
    Process* process = *processes;
    if(process->hProcess == INVALID_HANDLE_VALUE)
      continue;
    *(h++) = process->hProcess;
    *(p++) = process;
    if(h >= hend)
      break;
  }
  DWORD dw = WaitForMultipleObjects((DWORD)(h - handles), handles, FALSE, INFINITE);
  ssize pIndex;
  if(dw <= WAIT_OBJECT_0 || (pIndex = dw - (WAIT_OBJECT_0 + 1)) >= p - processMap)
    return 0;
  return processMap[pIndex];
#else
  VERIFY(pthread_mutex_lock(&ProcessFramework::mutex) == 0);
  switch(ProcessFramework::signaled)
  {
  case 0:
    if(count == 0)
    {
      ProcessFramework::waitState = 1;
      for(;;)
      {
        VERIFY(pthread_cond_wait(&ProcessFramework::condition, &ProcessFramework::mutex) == 0);
        if(ProcessFramework::signaled)
        {
          ASSERT(ProcessFramework::signaled < 0);
          ProcessFramework::signaled = 0;
          ProcessFramework::waitState = 0;
          VERIFY(pthread_mutex_unlock(&ProcessFramework::mutex) == 0);
          return 0;
        }
      }
    }
    else
      ProcessFramework::waitState = 2;
    break;
  case -1:
    ProcessFramework::signaled = 0;
    VERIFY(pthread_mutex_unlock(&ProcessFramework::mutex) == 0);
    return 0;
  default:
    {
      int status;
      pid_t pid = waitpid(ProcessFramework::signaled, &status, 0);
      if(pid == ProcessFramework::signaled)
        ProcessFramework::signaled = 0;
      ProcessFramework::waitState = 0;
    }
    VERIFY(pthread_mutex_unlock(&ProcessFramework::mutex) == 0);
    return 0;
  }
  VERIFY(pthread_mutex_unlock(&ProcessFramework::mutex) == 0);
  siginfo_t sigInfo;
  pid_t ret = waitid(P_ALL, 0, &sigInfo, WEXITED | WNOWAIT);
  if(ret != -1)
  {
    pid_t pid = sigInfo.si_pid;
    for(Process** end = processes + count; processes < end; ++processes)
      if(pid == (pid_t)(*processes)->pid)
        return *processes;
    VERIFY(pthread_mutex_lock(&ProcessFramework::mutex) == 0);
    if(pid == ProcessFramework::signaled)
    {
        int status;
        pid_t pid = waitpid(ProcessFramework::signaled, &status, 0);
        if(pid == ProcessFramework::signaled)
          ProcessFramework::signaled = 0;
    }
    ProcessFramework::waitState = 0;
    VERIFY(pthread_mutex_unlock(&ProcessFramework::mutex) == 0);
  }
  return 0;
#endif
}

void Process::interrupt()
{
#ifdef _WIN32
  EnterCriticalSection(&ProcessFramework::criticalSection);
  if(ProcessFramework::hInterruptEvent == INVALID_HANDLE_VALUE)
    ProcessFramework::hInterruptEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
   SetEvent(ProcessFramework::hInterruptEvent);
  LeaveCriticalSection(&ProcessFramework::criticalSection);
#else
  VERIFY(pthread_mutex_lock(&ProcessFramework::mutex) == 0);
  if(!ProcessFramework::signaled)
    switch(ProcessFramework::waitState)
    {
    case 2:
      {
        pid_t pid = vfork();
        switch(pid)
        {
        case 0:
          _exit(0);
        case -1:
          break;
        default:
          ProcessFramework::signaled = pid;
        }
      }
      break;
    case 1:
      ProcessFramework::signaled = -1;
      VERIFY(pthread_cond_signal(&ProcessFramework::condition) == 0);
      break;
    default:
      ProcessFramework::signaled = -1;
    }
  VERIFY(pthread_mutex_unlock(&ProcessFramework::mutex) == 0);
#endif
}


String Process::getExecutablePath()
{
#ifdef _WIN32
  TCHAR path[MAX_PATH + 1];
  DWORD len = GetModuleFileName(NULL, path, sizeof(path) / sizeof(*path));
  if (len != sizeof(path) / sizeof(*path))
    return String::fromCString(path, len);
  Buffer buffer;
  buffer.resize((MAX_PATH << 1) * sizeof(TCHAR));
  for (;;)
  {
    DWORD len = GetModuleFileName(NULL, (TCHAR*)(byte*)buffer, (DWORD)buffer.size());
    if (len != buffer.size() / sizeof(TCHAR))
      return String::fromCString(path, len);
    buffer.resize(buffer.size() << 1);
  }
#else
  char procFilePath[32];
  sprintf(procFilePath, "/proc/%d/exe", getpid());
  char path[PATH_MAX + 1];
  ssize_t len = readlink(procFilePath, path, sizeof(path));
  if (len == -1)
    return String();
  return String::fromCString(path, len);
#endif
}
