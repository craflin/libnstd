
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

  // load path env
  List<String> searchPaths;
  {
    char* pathVar = getenv("PATH");
    for(const char* str = pathVar; *str;)
    {
      const char* end = strchr(str, ':');
      if(end)
      {
        if(end > str)
          searchPaths.append(String(str, end - str));
        ++end;
        str = end;
      }
      else
      {
        searchPaths.append(String(str, String::length(str)));
        break;
      }
    }
  }


  // find executable
  String program;
  if(!command.isEmpty())
    program = command.front();
  String programPath = program;
  if(((const char*)programPath)[0] != '/')
    for(List<String>::Iterator i = searchPaths.begin(), end = searchPaths.end(); i != end; ++i)
    {
      String testPath = *i;
      testPath.append('/');
      testPath.append(program);
      if(File::exists(testPath))
      {
        programPath = testPath;
        break;
      }
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

    const char** envp = (const char**)environ;
    const char* executable = programPath;
    if(execve(executable, (char* const*)argv, (char* const*)envp) == -1)
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
  return false;
#else
  // todo
  return 0;
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
  // todo
  return -1;
#endif
}

ssize_t Process::read(void_t* buffer, size_t length, uint_t& streams)
{
#ifdef _WIN32
  HANDLE handles[2];
  DWORD handleCount = 0;
  if(streams & stdoutStream)
    handles[handleCount++] = hStdOutRead;
  if(streams & stderrStream)
    handles[handleCount++] = hStdErrRead;
  DWORD dw = WaitForMultipleObjects(handleCount, handles, FALSE, INFINITE);
  if(dw < WAIT_OBJECT_0 || dw >= WAIT_OBJECT_0 + handleCount)
    return -1;
  DWORD i;
  if(!ReadFile(handles[dw - WAIT_OBJECT_0], buffer, length, &i, NULL))
    return -1;
  return i;
#else
  // todo
  return -1;
#endif
}

ssize_t Process::write(const void_t* buffer, size_t length)
{
#ifdef _WIN32
  // todo
  DWORD i;
  if(!WriteFile(hStdInWrite, buffer, length, &i, NULL))
    return -1;
  return i;
#else
  // todo
  return -1;
#endif
}
