
#ifdef _WIN32
#include <windows.h>
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
#endif

#include <nstd/Debug.h>
#include <nstd/List.h>
#ifndef _WIN32
#include <nstd/File.h>
#endif
#include <nstd/Process.h>

Process::Process()
{
#ifdef _WIN32
  ASSERT(sizeof(hProcess) >= sizeof(HANDLE));
  hProcess = INVALID_HANDLE_VALUE;
  hStdOutRead = INVALID_HANDLE_VALUE;
  hStdErrRead = INVALID_HANDLE_VALUE;
  hStdInWrite = INVALID_HANDLE_VALUE;
#else
  pid = 0;
  fdStdOutRead = 0;
  fdStdErrRead = 0;
  fdStdInWrite = 0;
#endif
}

Process::~Process()
{
  uint32_t exitCode;
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

uint32_t Process::start(const String& commandLine)
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

  if(!CreateProcess(NULL, (tchar_t*)args, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    return 0;

  CloseHandle(pi.hThread);

  ASSERT(pi.hProcess);
  hProcess = pi.hProcess;

  return pi.dwProcessId;

#else
  if(pid)
  {
    errno = EINVAL;
    return false;
  }

  // split commandLine into args
  List<String> command;
  {
    String arg;
    for(const tchar_t* p = commandLine; *p;)
      switch(*p)
      {
      case _T('"'):
        for(++p; *p;)
          switch(*p)
          {
          case _T('"'):
            ++p;
            break;
          case _T('\\'):
            if(p[1] == _T('"'))
            {
              arg.append(_T('"'));
              p += 2;
            }
            break;
          default:
            arg.append(*(p++));
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
    if(!arg.isEmpty())
      command.append(arg);
  }

  // start process
  int r = vfork();
  if(r == -1)
    return 0;
  else if(r != 0) // parent
  {
    pid = r;
    return r;
  }
  else // child
  {
    const char** argv = (const char**)alloca(sizeof(const char*) * (command.size() + 1));
    int i = 0;
    for(List<String>::Iterator j = command.begin(), end = command.end(); j != end; ++j)
      argv[i++] = *j;
    argv[i] = 0;

    const char* executable = i > 0 ? argv[0] : "";
    if(execvp(executable, (char* const*)argv) == -1)
    {
      fprintf(stderr, "%s: %s\n", executable, strerror(errno));
      _exit(EXIT_FAILURE);
    }
    ASSERT(false); // unreachable
    return 0;
  }
#endif
}

bool_t Process::kill()
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
  pid = 0;
  if(fdStdOutRead)
  {
    close(fdStdOutRead);
    fdStdOutRead = 0;
  }
  if(fdStdErrRead)
  {
    close(fdStdErrRead);
    fdStdErrRead = 0;
  }
  if(fdStdInWrite)
  {
    close(fdStdInWrite);
    fdStdInWrite = 0;
  }
  return true;
#endif
}

bool_t Process::join(uint32_t& exitCode)
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
  exitCode = (uint32_t)dwExitCode;
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
  pid = 0;
  if(fdStdOutRead)
  {
    close(fdStdOutRead);
    fdStdOutRead = 0;
  }
  if(fdStdErrRead)
  {
    close(fdStdErrRead);
    fdStdErrRead = 0;
  }
  if(fdStdInWrite)
  {
    close(fdStdInWrite);
    fdStdInWrite = 0;
  }
  return true;
#endif
}

uint32_t Process::getCurrentProcessId()
{
#ifdef _WIN32
  return (uint32_t)GetCurrentProcessId();
#else
  return (uint32_t)getpid();
#endif
}

bool_t Process::open(const String& commandLine, uint_t streams)
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
  if(streams & stderrStream)
  {
   if(!CreatePipe(&hStdErrRead, &si.hStdError, &sa, 0) ||
      !SetHandleInformation(hStdErrRead, HANDLE_FLAG_INHERIT, 0))
      goto error;
  }
  if(streams & stdinStream)
  {
   if(!CreatePipe(&si.hStdInput, &hStdInWrite, &sa, 0) ||
      !SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0))
      goto error;
  }

  {
    String args(commandLine);

    if(!CreateProcess(NULL, (tchar_t*)args, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
      goto error;
  }

  CloseHandle(pi.hThread);

  if(si.hStdOutput != INVALID_HANDLE_VALUE)
    CloseHandle(si.hStdOutput);
  if(si.hStdError != INVALID_HANDLE_VALUE)
    CloseHandle(si.hStdError);
  if(si.hStdInput != INVALID_HANDLE_VALUE)
    CloseHandle(si.hStdInput);

  ASSERT(pi.hProcess);
  hProcess = pi.hProcess;
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
  if(si.hStdOutput != INVALID_HANDLE_VALUE)
    CloseHandle(si.hStdOutput);
  if(si.hStdError != INVALID_HANDLE_VALUE)
    CloseHandle(si.hStdError);
  if(si.hStdInput != INVALID_HANDLE_VALUE)
    CloseHandle(si.hStdInput);
  SetLastError(err);
  return false;
#else
  if(pid)
  {
    errno = EINVAL;
    return false;
  }

  // split commandLine into args
  List<String> command;
  {
    String arg;
    for(const tchar_t* p = commandLine; *p;)
      switch(*p)
      {
      case _T('"'):
        for(++p; *p;)
          switch(*p)
          {
          case _T('"'):
            ++p;
            break;
          case _T('\\'):
            if(p[1] == _T('"'))
            {
              arg.append(_T('"'));
              p += 2;
            }
            break;
          default:
            arg.append(*(p++));
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
    if(!arg.isEmpty())
      command.append(arg);
  }

  // create pipes
  int stdoutFds[2] = {};
  int stderrFds[2] = {};
  int stdinFds[2] = {};
  if(streams & stdoutStream)
  {
    if(pipe(stdoutFds) != 0)
      goto error;
  }
  if(streams & stderrStream)
  {
    if(pipe(stderrFds) != 0)
      goto error;
  }
  if(streams & stdinStream)
  {
    if(pipe(stdinFds) != 0)
      goto error;
  }

  // start process
  {
    int r = vfork();
    if(r == -1)
      return false;
    else if(r != 0) // parent
    {
      pid = r;

      if(stdoutFds[1])
        close(stdoutFds[1]);
      if(stderrFds[1])
        close(stderrFds[1]);
      if(stdinFds[0])
        close(stdinFds[0]);

      fdStdOutRead = stdoutFds[0];
      fdStdErrRead = stderrFds[0];
      fdStdInWrite = stderrFds[1];

      return true;
    }
    else // child
    {
      if(stdoutFds[1])
      {
        dup2(stdoutFds[1], STDOUT_FILENO);
        close(stdoutFds[1]);
      }
      if(stderrFds[1])
      {
        dup2(stderrFds[1], STDERR_FILENO);
        close(stderrFds[1]);
      }
      if(stdinFds[0])
      {
        dup2(stdinFds[0], STDIN_FILENO);
        close(stdinFds[0]);
      }

      if(stdoutFds[0])
        close(stdoutFds[0]);
      if(stderrFds[0])
        close(stderrFds[0]);
      if(stdinFds[1])
        close(stdinFds[1]);

      const char** argv = (const char**)alloca(sizeof(const char*) * (command.size() + 1));
      int i = 0;
      for(List<String>::Iterator j = command.begin(), end = command.end(); j != end; ++j)
        argv[i++] = *j;
      argv[i] = 0;

      const char* executable = i > 0 ? argv[0] : "";
      if(execvp(executable, (char* const*)argv) == -1)
      {
        fprintf(stderr, "%s: %s\n", executable, strerror(errno));
        _exit(EXIT_FAILURE);
      }
      ASSERT(false); // unreachable
      return false;
    }
  }
error:
  int err = errno;
  if(stdoutFds[0])
  {
    close(stdoutFds[0]);
    close(stdoutFds[1]);
  }
  if(stderrFds[0])
  {
    close(stderrFds[0]);
    close(stderrFds[1]);
  }
  if(stdinFds[0])
  {
    close(stdinFds[0]);
    close(stdinFds[1]);
  }
  errno = err;
  return false;
#endif
}

ssize_t Process::read(void_t* buffer, size_t length)
{
#ifdef _WIN32
  DWORD i;
  if(!ReadFile(hStdOutRead, buffer, length, &i, NULL))
    return -1;
  return i;
#else
  return ::read(fdStdOutRead, buffer, length);
#endif
}

ssize_t Process::read(void_t* buffer, size_t length, uint_t& streams)
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
  DWORD i;
  HANDLE readHandle = handles[dw - WAIT_OBJECT_0];
  streams = readHandle == hStdOutRead ? stdoutStream : stderrStream;
  if(!ReadFile(readHandle, buffer, length, &i, NULL))
    return -1;
  return i;
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

ssize_t Process::write(const void_t* buffer, size_t length)
{
#ifdef _WIN32
  DWORD i;
  if(!WriteFile(hStdInWrite, buffer, length, &i, NULL))
    return -1;
  return i;
#else
  return ::write(fdStdInWrite, buffer, length);
#endif
}
