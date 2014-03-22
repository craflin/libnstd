
//#include <malloc.h>
#ifdef _WIN32
#include <windows.h>
#else
//#include <unistd.h>
//#include <cerrno>
//#include <cstdlib>
//#include <sys/types.h>
//#include <sys/wait.h>
//#include <cstdio>
//#include <cstring>
//#include <sys/utsname.h> // uname
#endif

#include <nstd/Debug.h>
#include <nstd/List.h>
//#include <nstd/Mutex.h>
//#include <nstd/File.h>
//#include <nstd/Buffer.h>
#include <nstd/Process.h>

Process::Process()
{
#ifdef _WIN32
  ASSERT(sizeof(hProcess) >= sizeof(HANDLE));
  hProcess = INVALID_HANDLE_VALUE;
#else
  pid = 0;
  exitCode = 1;
#endif
}

Process::~Process()
{
#ifdef _WIN32
  ASSERT(hProcess ==  INVALID_HANDLE_VALUE);
  if(hProcess != INVALID_HANDLE_VALUE)
    CloseHandle((HANDLE)hProcess);
#else
  ASSERT(pid ==  0);
#endif
}

bool Process::isRunning() const
{
#ifdef _WIN32
  return hProcess != INVALID_HANDLE_VALUE;
#else
  return pid != 0;
#endif
}

/*
class Word
{
public:
  enum Flags
  {
    quotedFlag = 0x01,
  };

public:
  Word(const String& word, uint32_t flags) : word(word), flags(flags) {}

  static void split(const String& text, List<Word>& words)
  {
    const tchar_t* str = text;
    for(;;)
    {
      while(String::isSpace(*str))
        ++str;
      if(!*str)
        break;
      if(*str == '"')
      {
        ++str;
        const char* end = str;
        for(; *end; ++end)
          if(*end == '\\')
          {
            switch(end[1] == '"')
            ++end;
          else if(*end == '"')
            break;
        if(end > str) // TODO: read escaped spaces as ordinary spaces?
          words.append(text.substr(str - (const char_t*)text, end - str));
        str = end;
        if(*str)
          ++str; // skip closing '"'
      }
      else
      {
        const char* end = str;
        for(; *end; ++end)
          if(isspace(*(unsigned char*)end))
            break;
        // TODO: read escaped spaces as ordinary spaces
        words.append(text.substr(str - (const char_t*)text, end - str));
        str = end;
      }
    }
  }

  void append(const List<Word>& words, String& text)
  {
    if(words.isEmpty())
      return;

    int totalLen = words.getSize() * 3;
    for(const List<Word>::Node* i = words.getFirst(); i; i = i->getNext())
      totalLen += i->data.getLength();
    text.setCapacity(totalLen + 16);

    const List<Word>::Node* i = words.getFirst();
    //const List<Word>::Node* previousWord = i;
    i->data.appendTo(text);
    for(i = i->getNext(); i; i = i->getNext())
    {
      text.append(/ *previousWord->data.terminated ? '\n' : * /' ');
      i->data.appendTo(text);
      //previousWord = i;
    }
  }

private:
  String word;
  uint32_t flags;
}
  */

uint32_t Process::start(const String& commandLine)
{
#ifdef _WIN32
//  String program(MAX_PATH);
//  for(const tchar_t* p = commandLine; *p;)
//    switch(*p)
//    {
//    case _T('"'):
//      for(++p; *p;)
//        switch(*p)
//        {
//        case _T('"'):
//          ++p;
//          break;
//        case _T('\\'):
//          if(p[1] == _T('"'))
//          {
//            program.append(_T('"'));
//            p += 2;
//          }
//          break;
//        default:
//          program.append(*(p++));
//      }
//      break;
//    case _T(' '):
//      goto done;
//    default:
//      program.append(*(p++));
//    }
//done: ;



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
#error todo
#endif
}

#if 0


unsigned int Process::start(const String& rawCommandLine)
{
  // split commands into words
  List<String> command;
  wordSplit(rawCommandLine, command);

  // separate leading environment variables and the command line
  HashMap<String, String> environmentVariables;
  for(List<String>::Iterator envNode = command.begin(), end = command.end(); envNode != end; ++envNode)
  {
    const char* data = *envNode;
    const char* sep = strchr(data, '=');
    if(sep)
    {
      // load list of existing existing variables
      if(environmentVariables.isEmpty())
        getEnvironmentVariables(environmentVariables);

      // add or override a variable
      String key(data, sep - data);
      const char_t* valuep = sep + 1;
      String value(valuep, envNode->length() - (valuep - data));
      environmentVariables.append(key, value);

      //
      command.removeFront();
    }
    else
      break;
  }

#ifdef _WIN32
  struct Executable
  {
    static bool fileComplete(const String& searchName, bool testExtensions, String& result)
    {
      if(File::exists(searchName))
      {
        result = searchName;
        return true;
      }
      if(testExtensions)
      {
        String testPath = searchName;
        testPath.append(".exe");
        if(File::exists(testPath))
        {
          result = testPath;
          return true;
        }
        testPath.resize(searchName.length());
        testPath.append(".com");
        if(File::exists(testPath))
        {
          result = testPath;
          return true;
        }
      }
      return false;
    }

    static const List<String>& getPathEnv()
    {
      static List<String> searchPaths;
      static bool loaded = false;
      if(!loaded)
      {
        Buffer pathVar;
        pathVar.resize(32767);
        GetEnvironmentVariable("PATH", (char_t*)(byte_t*)pathVar, 32767);
        for(const char* str = (char_t*)(byte_t*)pathVar; *str;)
        {
          const char* end = strchr(str, ';');
          if(end)
          {
            if(end > str)
              searchPaths.append(String(str, end - str));
            ++end;
            str = end;
          }
          else
          {
            searchPaths.append(String(str, -1));
            break;
          }
        }
        loaded = true;
      }
      return searchPaths;
    }

    static bool resolveSymlink(const String& fileName, String& result)
    {
      String cygwinRoot = File::dirname(File::dirname(fileName));
      result = fileName;
      bool success = false;
      for(;;)
      {
        File file;
        if(!file.open(result))
          return success;
        const int len = 12 + MAX_PATH * 2 + 2;
        char buffer[len];
        int i = file.read(buffer, len);
        if(i < 12 || strncmp(buffer, "!<symlink>\xff\xfe", 12) != 0)
          return success;
        i &= ~1;
        wchar_t* wdest = (wchar_t*)(buffer + 12);
        wdest[(i - 12) >> 1] = 0;
        String dest;
        dest.printf("%S", wdest);
        if(strncmp(dest, "/usr/bin/", 9) == 0)
        {
          result = cygwinRoot;
          result.append(dest.substr(4));
        }
        else if(((const char_t*)dest)[0] == '/')
        {
          result = cygwinRoot;
          result.append(dest);
        }
        else
        {
          result = File::dirname(result);
          result.append('/');
          result.append(dest);
        }
        success = true;
      }
      return false;
    }

    static String find(const String& program)
    {
      String result = program;
      bool testExtensions = File::getExtension(program).isEmpty();
      // check whether the given path is absolute
      if(program.getData()[0] == '/' || (program.getLength() > 2 && program.getData()[1] == ':'))
      { // absolute
        fileComplete(program, testExtensions, result);
      }
      else
      { // try each search path
        const List<String>& searchPaths = getPathEnv();
        for(const List<String>::Node* i = searchPaths.getFirst(); i; i = i->getNext())
        {
          String testPath = i->data;
          testPath.append('\\');
          testPath.append(program);
          if(fileComplete(testPath, testExtensions, result))
          {
            if(strncmp(program.getData(), "../", 3) == 0 || strncmp(program.getData(), "..\\", 3) == 0)
              result = File::simplifyPath(result);
            break;
          }
        }
      }
      return result;
    }
  };

  String program, programPath;
  static Map<String, String> cachedProgramPaths;
  bool cachedProgramPath = false;
  if(command.isEmpty())
    cachedProgramPath = true;
  else
  {
    program = command.getFirst()->data;
    const Map<String, String>::Node* i = cachedProgramPaths.find(program);
    if(i)
    {
      programPath = i->data;
      cachedProgramPath = true;
    }
    else
      programPath = Executable::find(program);
  }

  String commandLine;
  if(!command.isEmpty())
  {
    if(strncmp(command.getFirst()->data.getData(), "../", 3) == 0 || strncmp(command.getFirst()->data.getData(), "..\\", 3) == 0)
      command.getFirst()->data = programPath;
    Word::append(command, commandLine);
  }
  commandLine.setCapacity(commandLine.getLength()); // enforce detach

  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  char* envblock = 0;
  if(!environmentVariables.isEmpty())
  {
    int envlen = 1;
    const String** envStrings = (const String**)_alloca(environmentVariables.getSize() * sizeof(String*));
    const String** j = envStrings;
    for(Map<String, String>::Node* i = environmentVariables.getFirst(); i; i = i->getNext())
    {
      envlen += i->data.getLength() + 1;
      *(j++) = &i->data;
    }
    struct SortFunction
    {
      static int cmp(const void* a, const void* b)
      {
        return _stricmp((*(const String**)a)->getData(), (*(const String**)b)->getData());
      }
    };
    qsort(envStrings, environmentVariables.getSize(), sizeof(char*), SortFunction::cmp);

    envblock = (char*)_alloca(envlen);
    char* p = envblock; 
    for(const String** i = envStrings, ** end = envStrings + environmentVariables.getSize(); i < end; ++i)
    {
      int varlen = (*i)->getLength() + 1;
      memcpy(p, (*i)->getData(), varlen);
      p += varlen;
    }
    *p = '\0';
  }

  if(!CreateProcess(programPath.getData(), (char*)commandLine.getData(), NULL, NULL, FALSE, 0, envblock, NULL, &si, &pi))
  {
    DWORD lastError = GetLastError();
    if(!programPath.isEmpty())
    {
      String resolvedSymlink;
      if(Executable::resolveSymlink(programPath, resolvedSymlink))
      {
        programPath = resolvedSymlink;
        if(CreateProcess(programPath.getData(), (char*)commandLine.getData(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
          goto success;
        else
          lastError = GetLastError();
      }
    }

    if(!cachedProgramPath)
      cachedProgramPaths.append(program, programPath);

    SetLastError(lastError);
    return 0;
  }
success:

  if(!cachedProgramPath)
    cachedProgramPaths.append(program, programPath);

  CloseHandle(pi.hThread);

  ASSERT(pi.hProcess);
  hProcess = pi.hProcess;

  return pi.dwProcessId;
#else

  struct Executable
  {
    static const List<String>& getPathEnv()
    {
      static List<String> searchPaths;
      static bool loaded = false;
      if(!loaded)
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
            searchPaths.append(String(str, -1));
            break;
          }
        }
        loaded = true;
      }
      return searchPaths;
    }

#ifdef __CYGWIN__
    static bool fileComplete(const String& searchName, bool testExtensions, String& result)
    {
      if(File::exists(searchName))
      {
        result = searchName;
        return true;
      }
      if(testExtensions)
      {
        String testPath = searchName;
        testPath.append(".exe");
        if(File::exists(testPath))
        {
          result = testPath;
          return true;
        }
        testPath.setLength(searchName.getLength());
        testPath.append(".com");
        if(File::exists(testPath))
        {
          result = testPath;
          return true;
        }
      }
      return false;
    }
    static String find(const String& program)
    {
      String result = program;
      bool testExtensions = File::getExtension(program).isEmpty();
      // check whether the given path is absolute
      if(program.getData()[0] == '/')
      { // absolute
        fileComplete(program, testExtensions, result);
      }
      else
      { // try each search path
        const List<String>& searchPaths = Executable::getPathEnv();
        for(const List<String>::Node* i = searchPaths.getFirst(); i; i = i->getNext())
        {
          String testPath = i->data;
          testPath.append('/');
          testPath.append(program);
          if(fileComplete(testPath, testExtensions, result))
            break;
        }
      }
      return result;
    }
#else
    static String find(const String& program)
    {
      String result = program;
      // check whether the given path is absolute
      if(program.getData()[0] == '/')
      { // absolute
        return result;
      }
      else
      { // try each search path
        const List<String>& searchPaths = Executable::getPathEnv();
        for(const List<String>::Node* i = searchPaths.getFirst(); i; i = i->getNext())
        {
          String testPath = i->data;
          testPath.append('/');
          testPath.append(program);
          if(File::exists(testPath))
          {
            result = testPath;
            break;
          }
        }
      }
      return result;
    }
#endif
  };

  String program, programPath;
  static Map<String, String> cachedProgramPaths;
  if(!command.isEmpty())
  {
    program = command.getFirst()->data;
    const Map<String, String>::Node* i = cachedProgramPaths.find(program);
    if(i)
      programPath = i->data;
    else
    {
      programPath = Executable::find(program);
      cachedProgramPaths.append(program, programPath);
    }
  }

  int r = vfork();
  if(r == -1)
    return 0;
  else if(r != 0) // parent
  {
    pid = r;
    runningProcesses.append(pid, this);
    return r;
  }
  else // child
  {
    const char** argv = (const char**)alloca(sizeof(const char*) * (command.getSize() + 1));
    int i = 0;
    for(const List<Word>::Node* j = command.getFirst(); j; j = j->getNext())
      argv[i++] = j->data.getData();
    argv[i] = 0;

    const char** envp = (const char**)environ;
    if(!environmentVariables.isEmpty())
    {
      envp = (const char**)alloca(sizeof(const char*) * (environmentVariables.getSize() + 1));
      int i = 0;
      for(const Map<String, String>::Node* j = environmentVariables.getFirst(); j; j = j->getNext())
        envp[i++] = j->data.getData();
      envp[i] = 0;
    }

    const char* executable = programPath.getData();
    if(execve(executable, (char* const*)argv, (char* const*)envp) == -1)
    {
      fprintf(stderr, "%s: %s\n", executable, Error::getString().getData());
      _exit(EXIT_FAILURE);
    }
    ASSERT(false); // unreachable
    return 0;
  }
#endif
}
#endif

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
  return true;
#else
  if(!pid)
  {
    errno = EINVAL;
    return 0;
  }
  pid = 0;
  return exitCode;
#endif
}

//static Mutex evnMutex;
//void_t Process::getEnvironmentVariables(HashMap<String, String>& vars)
//{
//  evnMutex.lock();
//  static bool loaded = false;
//  static HashMap<String, String> environmentVariables;
//  if(!loaded)
//  {
//#ifdef _WIN32
//    char* existingStrings = GetEnvironmentStrings();
//    for(const char* p = existingStrings; *p;)
//    {
//      int len = strlen(p);
//      const char* sep = strchr(p, '=');
//      if(sep)
//      {
//        String key(p, sep - p);
//        const char_t* valuep = sep + 1;
//        String value(valuep, len - (valuep - p));
//        environmentVariables.append(key, value);
//      }
//      p += len + 1;
//    }
//    FreeEnvironmentStrings(existingStrings);
//#else
//    for(char** pp = environ; *pp; ++pp)
//    {
//      const char* p = *pp;
//      const char* sep = strchr(p, '=');
//      if(sep)
//      {
//        String key(p, sep - p);
//        const char_t* valuep = sep + 1;
//        String value(valuep, strlen(valuep));
//        environmentVariables.append(key, value);
//      }
//    }
//#endif
//  }
//  vars = environmentVariables;
//  evnMutex.unlock();
//}
