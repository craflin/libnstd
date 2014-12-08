
#include <cstdarg>
#include <cstdio>
#ifdef _MSC_VER
#include <tchar.h>
#endif
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
//#include <conio.h>
#else
#include <unistd.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cctype>
#include <cstdlib>
#endif

#include <nstd/Debug.h>
#include <nstd/Console.h>
#include <nstd/Buffer.h>

int_t Console::print(const tchar_t* str)
{
#ifdef _MSC_VER
  return _fputts(str, stdout);
#else
  return fputs(str, stdout);
#endif
}

int_t Console::printf(const tchar_t* format, ...)
{
  va_list ap;
  va_start(ap, format);
#ifdef _UNICODE
  int_t result = vwprintf(format, ap);
#else
  int_t result = vprintf(format, ap);
#endif
  va_end(ap);
  return result;
}

int_t Console::error(const tchar_t* str)
{
#ifdef _MSC_VER
  return _fputts(str, stderr);
#else
  return fputs(str, stderr);
#endif
}

int_t Console::errorf(const tchar_t* format, ...)
{
  va_list ap;
  va_start(ap, format);
#ifdef _UNICODE
  int_t result = vfwprintf(stderr, format, ap);
#else
  int_t result = vfprintf(stderr, format, ap);
#endif
  va_end(ap);
  return result;
}

#ifdef _WIN32
static BOOL CreatePipeEx(LPHANDLE lpReadPipe, LPHANDLE lpWritePipe, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD nSize, DWORD dwReadMode, DWORD dwWriteMode)
{
  if((dwReadMode | dwWriteMode) & (~FILE_FLAG_OVERLAPPED))
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }
  if(nSize == 0)
    nSize = 4096;
  String name;
  static volatile unsigned long pipeCount = 0;
  unsigned long pipeId = InterlockedIncrement(&pipeCount);
  name.printf(_T("\\\\.\\pipe\\MyAnon.%08x.%08x"), (unsigned int)GetCurrentProcessId(), (unsigned int)pipeId);
  HANDLE hRead = CreateNamedPipe((const tchar_t*)name, PIPE_ACCESS_INBOUND | dwReadMode, PIPE_TYPE_BYTE | PIPE_WAIT, 1, nSize, nSize, 120 * 1000, lpSecurityAttributes);
  if(!hRead)
    return FALSE;
  HANDLE hWrite = CreateFile((const tchar_t*)name, GENERIC_WRITE, 0, lpSecurityAttributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | dwWriteMode, NULL);
  if(hWrite == INVALID_HANDLE_VALUE)
  {
    DWORD dwError = GetLastError();
    CloseHandle(hRead);
    SetLastError(dwError);
    return FALSE;
  }
  *lpReadPipe = hRead;
  *lpWritePipe = hWrite;
  return TRUE;
}
#endif

class ConsolePromptPrivate
{
public:

#ifndef _WIN32
  static void restoreTermMode()
  {
    if(originalTermiosValid)
    {
      VERIFY(tcsetattr(originalStdout, TCSAFLUSH, &originalTermios) == 0);
      originalTermiosValid = false;
    }
  }
#endif

  ConsolePromptPrivate() : valid(false)
  {
#ifdef _WIN32
    if(!GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &consoleMode))
      return; // no tty?
    valid = true;

    // redirect stdout and stderr to pipe
    VERIFY(CreatePipeEx(&hStdOutRead, &hStdOutWrite, NULL, 0, FILE_FLAG_OVERLAPPED, 0));
    originalStdout = _dup(_fileno(stdout));
    newStdout = _open_osfhandle((intptr_t)hStdOutWrite, _O_TEXT);
    ASSERT(newStdout != -1);
    VERIFY(_dup2(newStdout, _fileno(stdout)) == 0);
    VERIFY(setvbuf(stdout, NULL, _IONBF, 0) == 0);
    hOriginalStdOut = (HANDLE)_get_osfhandle(originalStdout);
    ASSERT(hOriginalStdOut != INVALID_HANDLE_VALUE);

    // initialize overlapped reading of hStdOutRead
    ZeroMemory(&overlapped, sizeof(overlapped));
    VERIFY(overlapped.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL));
    DWORD read;
    while(ReadFile(hStdOutRead, stdoutBuffer, sizeof(stdoutBuffer), &read, &overlapped))
    {
      DWORD written;
      VERIFY(WriteFile(hOriginalStdOut, stdoutBuffer, read, &written, NULL));
      ASSERT(written == read);
    }

    // get screen width
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    VERIFY(GetConsoleScreenBufferInfo(hOriginalStdOut, &csbi));
    stdoutScreenWidth = csbi.dwSize.X;
#else
    if(originalStdout != -1)
      return; // yout should create more than a single instance of this class
    if(!isatty(STDIN_FILENO))
      return;
    valid = true;

    originalStdout = eventfd(0, EFD_CLOEXEC); // create new file descriptor with O_CLOEXEC.. is there a better way to do this?
    VERIFY(dup3(STDOUT_FILENO, originalStdout, O_CLOEXEC) != -1); // create copy of STDOUT_FILENO
    int pipes[2];
    VERIFY(pipe2(pipes, O_CLOEXEC) == 0);
    stdoutRead = pipes[0];
    stdoutWrite = pipes[1];
    //::originalStdout = originalStdout;
    //pthread_atfork(0, 0, resetStdout);
    //arr? nun O_CLOEXEC von originalStdout entfernen?
    //VERIFY(dup3(stdoutWrite, STDOUT_FILENO, O_CLOEXEC) != -1);
    VERIFY(dup2(stdoutWrite, STDOUT_FILENO) != -1);
    VERIFY(setvbuf(stdout, NULL, _IONBF, 0) == 0);
    // todo: install SIGWINCH handler

    // save term mode
    if(!originalTermiosValid)
    {
      VERIFY(tcgetattr(originalStdout, &originalTermios) == 0);
      originalTermiosValid = true;
      atexit(restoreTermMode);
    }

    // get screen width
    {
      winsize ws;
      stdoutScreenWidth = 0;
      if(ioctl(originalStdout, TIOCGWINSZ, &ws) == 0)
        stdoutScreenWidth = ws.ws_col;
      if(stdoutScreenWidth == 0)
      {
        size_t x, y;
        getCursorPosition(x, y);
        writeConsole("\x1b[999C", 6);
        size_t newX, newY;
        getCursorPosition(newX, newY);
        if(newX > x)
        {
          String moveCmd;
          moveCmd.printf("\x1b[%dD",newX - x);
          writeConsole(moveCmd, moveCmd.length());
        }
        stdoutScreenWidth = newX + 1;
      }
    }

#endif
  }

  ~ConsolePromptPrivate()
  {
    if(!valid)
      return;
#ifdef _WIN32
    CancelIo(hStdOutRead);
    CloseHandle(overlapped.hEvent);
    _dup2(originalStdout, _fileno(stdout));
    _close(originalStdout); // this should close hOriginalStdOut
    _close(newStdout); // this should close hStdOutWrite
    CloseHandle(hStdOutRead);
#else
    // todo: uninstall SIGWINCH handler
    restoreTermMode();
    originalTermiosValid = false;
    VERIFY(dup2(originalStdout, STDOUT_FILENO) != -1);
    close(originalStdout);
    close(stdoutRead);
    close(stdoutWrite);
    originalStdout = -1;
#endif
  }

  void_t getCursorPosition(size_t& x, size_t& y)
  {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    VERIFY(GetConsoleScreenBufferInfo(hOriginalStdOut, &csbi));
    x = csbi.dwCursorPosition.X;
    y = csbi.dwCursorPosition.Y;
#else
    VERIFY(write(originalStdout, "\x1b[6n", 4) == 4);
    char_t buffer[64];
    size_t bufferUsed = 0;
    for(;;)
    {
    nextRead:
      ssize_t len = read(STDIN_FILENO, buffer - bufferUsed, sizeof(buffer) - bufferUsed- 1);
      VERIFY(len != -1);
      len += bufferUsed;
      buffer[len] = '\0';
      char* search = buffer;
      for(;;)
      {
        char* response = strstr(search, "\x1b");
        if(!response)
        {
          bufferedInput.append((byte_t*)buffer, len);
          goto nextRead;
        }
        char* responseEnd = response + 1;
        if(*responseEnd == '[')
          ++responseEnd;
        while(*responseEnd && !isalpha(*responseEnd))
          ++responseEnd;
        if(!*responseEnd)
        {
          if(responseEnd - response < len)
          {
            search = responseEnd + 1;
            continue;
          }
          // incomplete response
          bufferedInput.append((byte_t*)buffer, response - buffer);
          bufferUsed = len - (response - buffer);
          Memory::move(buffer, response, bufferUsed);
          goto nextRead;
        }
        if(*responseEnd != 'R' || response[1] != '[')
        {
          search = responseEnd + 1;
          continue;
        }
        else
        {
          int ix, iy;
          VERIFY(sscanf(response + 2, "%d;%d", &iy, &ix) == 2);
          x = ix - 1;
          y = iy - 1;
          ++responseEnd;
          if(response != buffer)
            bufferedInput.append((byte_t*)buffer, response - buffer);
          if(responseEnd - response < len)
            bufferedInput.append((byte_t*)responseEnd, len - (responseEnd - response));
          return;
        }
      }
    }
#endif
  }

#ifdef _WIN32
  void_t setCursorPosition(size_t x, size_t y)
  {
    COORD pos = {(SHORT)x, (SHORT)y};
    VERIFY(SetConsoleCursorPosition(hOriginalStdOut, pos));
  }
#endif

  void_t moveCursorPosition(size_t from, ssize_t x)
  {
#ifdef _WIN32
    size_t to = from + x;
    size_t oldY = from / stdoutScreenWidth;
    //size_t oldX = from % stdoutScreenWidth;
    size_t newY = to / stdoutScreenWidth;
    size_t newX = to % stdoutScreenWidth;
    //if(newY != oldY)
    //{
      // the get/setCursorPosition combo resets the cursor blink timer!
      size_t cursorX, cursorY;
      getCursorPosition(cursorX, cursorY);
      setCursorPosition(newX, cursorY + ((ssize_t)newY - (ssize_t)oldY));
    //}
    //else if(newX == 0)
    //{
    //  tchar_t c = _T('\r');
    //  writeConsole(&c, 1);
    //}
    //else if(newX < oldX)
    //{
    //  size_t count = oldX - newX;
    //  String moveCmd(count);
    //  for(size_t i = 0; i < count; ++i)
    //    moveCmd.append(_T('\b'));
    //  writeConsole(moveCmd, moveCmd.length());
    //}
    //else if(newX > oldX)
    //{
    //  String buffer(prompt.length() + input.length());
    //  buffer.append(prompt);
    //  buffer.append(input);
    //  size_t offset = newY * width + oldX;
    //  size_t count = newX - oldX;
    //  writeConsole((const tchar_t*)buffer + offset, count);
    //}
#else
#endif
  }

  void_t writeConsole(const tchar_t* data, size_t len)
  {
#ifdef _WIN32
    DWORD written;
    VERIFY(WriteConsole(hOriginalStdOut, data, (DWORD)len, &written, NULL));
    ASSERT(written == (DWORD)len);
#else
    VERIFY(write(originalStdout, data, len) == (ssize_t)len);
#endif
  }

  void_t promptWrite(size_t offset = 0, const String& clearStr = String())
  {
    offset += offset / stdoutScreenWidth * 2;
    String buffer(prompt.length() + input.length() + clearStr.length());
    buffer.append(prompt);
    buffer.append(input);
    buffer.append(clearStr);
    String wrappedBuffer(buffer.length() + buffer.length() / stdoutScreenWidth * 2 + 2);
    for(size_t i = 0, len = buffer.length(); i < len; i += stdoutScreenWidth)
    {
      size_t lineEnd = len - i;
      if(lineEnd > stdoutScreenWidth)
        lineEnd = stdoutScreenWidth;
      wrappedBuffer.append((const tchar_t*)buffer + i, lineEnd);
      if(lineEnd == stdoutScreenWidth)
      {
        wrappedBuffer.append(_T('\r'));
        wrappedBuffer.append(_T('\n'));
      }
    }
    writeConsole((const tchar_t*)wrappedBuffer + offset, wrappedBuffer.length() - offset);
    if(caretPos < input.length() + clearStr.length())
      moveCursorPosition(prompt.length() + input.length() + clearStr.length(), -(ssize_t)(input.length() + clearStr.length() - caretPos));
#ifdef _WIN32
    else // enforce cursor blink reset
      moveCursorPosition(prompt.length() + input.length() + clearStr.length(), 0);
#endif
  }

  void_t promptInsert(tchar_t character)
  {
    if(caretPos != input.length())
    {
      String newInput(input.length() + 1);
      newInput.append(input.substr(0, caretPos));
      newInput.append(character);
      newInput.append(input.substr(caretPos));
      input = newInput;
    }
    else
      input.append(character);
    size_t oldCaretPos = caretPos;
    ++caretPos;
    promptWrite(prompt.length() + oldCaretPos);
  }

  void_t promptRemove()
  {
    if(!input.isEmpty() && caretPos > 0)
    {
      String newInput(input.length() - 1);
      newInput.append(input.substr(0, caretPos - 1));
      newInput.append(input.substr(caretPos));
      input = newInput;
      promptMoveLeft();
      promptWrite(prompt.length() + caretPos, _T(" "));
    }
  }

  void_t promptRemoveNext()
  {
    if(caretPos < input.length())
    {
      String newInput(input.length() - 1);
      newInput.append(input.substr(0, caretPos));
      newInput.append(input.substr(caretPos + 1));
      input = newInput;
      promptWrite(prompt.length() + caretPos, _T(" "));
    }
  }

  void_t promptMoveLeft()
  {
    if(caretPos > 0)
    {
      moveCursorPosition(prompt.length() + caretPos, -1);
      --caretPos;
    }
  }

  void_t promptHome()
  {
    if(caretPos > 0)
    {
      moveCursorPosition(prompt.length() + caretPos, -(ssize_t)caretPos);
      caretPos = 0;
    }
  }

  void_t promptMoveRight()
  {
    if(caretPos < input.length())
    {
      moveCursorPosition(prompt.length() + caretPos, 1);
      ++caretPos;
    }
  }

  void_t promptEnd()
  {
    if(caretPos < input.length())
    {
      moveCursorPosition(prompt.length() + caretPos, input.length() - caretPos);
      caretPos = input.length();
    }
  }

  void_t promptClear()
  {
    size_t bufferLen = prompt.length() + input.length();
    size_t additionalLines = bufferLen / stdoutScreenWidth;
    if(additionalLines)
    {
      moveCursorPosition(prompt.length() + caretPos, -(ssize_t)(caretPos + prompt.length()));
      String clearLine(stdoutScreenWidth + 2);
      for(size_t i = 0; i < stdoutScreenWidth; ++i)
        clearLine.append(_T(' '));
      clearLine.append(_T("\n\r"));
      String clearCmd(bufferLen + additionalLines * 2);
      for(size_t i = 0; i < additionalLines; ++i)
        clearCmd.append(clearLine);
      for(size_t i = 0, count = bufferLen - additionalLines * stdoutScreenWidth; i < count; ++i)
        clearCmd.append(_T(' '));
      writeConsole(clearCmd, clearCmd.length());
      moveCursorPosition(prompt.length() + input.length(), -(ssize_t)bufferLen);
    }
    else
    {
      String clearCmd(2 + bufferLen);
      clearCmd.append(_T('\r'));
      for(size_t i = 0, count = bufferLen; i < count; ++i)
        clearCmd.append(_T(' '));
      clearCmd.append(_T('\r'));
      writeConsole(clearCmd, clearCmd.length());
    }
  }

  void_t saveCursorPosition()
  {
    size_t x, y;
    getCursorPosition(x, y);
    stdoutCursorX = x;
    if(stdoutCursorX)
      writeConsole("\r\n", 2);
  }

  void_t restoreCursorPosition()
  {
    if(stdoutCursorX)
    {
#ifdef _WIN32
      CONSOLE_SCREEN_BUFFER_INFO csbi;
      VERIFY(GetConsoleScreenBufferInfo(hOriginalStdOut, &csbi));
      csbi.dwCursorPosition.X = stdoutCursorX;
      --csbi.dwCursorPosition.Y;
      VERIFY(SetConsoleCursorPosition(hOriginalStdOut, csbi.dwCursorPosition));
#else
      String moveCmd;
      moveCmd.printf("\x1b[A\r\x1b[%dC", stdoutCursorX);
      writeConsole(moveCmd, moveCmd.length());
#endif
    }
  }

  void_t restoreTerminalMode()
  {
#ifdef _WIN32
    VERIFY(SetConsoleMode(hOriginalStdOut, consoleMode));
#else
    VERIFY(tcsetattr(originalStdout, TCSAFLUSH, &originalTermios) == 0);
#endif
  }

  void_t enableTerminalRawMode()
  {
#ifdef _WIN32
    VERIFY(SetConsoleMode(hOriginalStdOut, ENABLE_PROCESSED_OUTPUT));
#else
    termios raw = originalTermios;
    cfmakeraw(&raw);
    raw.c_lflag |= ISIG;
    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0;
    VERIFY(tcsetattr(originalStdout, TCSAFLUSH, &raw) == 0);
#endif
  }

  String getLine(const String& prompt)
  {
    if(!valid)
    {
      // todo: print std and read line from stdin 
      return String();
    }

    enableTerminalRawMode();
    saveCursorPosition();

    // write prompt
    this->prompt = prompt;
    input.clear();
    inputComplete = false;
    caretPos = 0;
    promptWrite();

#ifdef _WIN32
    // wait for io
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE handles[] = {hStdIn, overlapped.hEvent};
    while(!inputComplete)
    {
      DWORD dw = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
      switch(dw)
      {
      case WAIT_OBJECT_0:
        {
          INPUT_RECORD records[1];
          DWORD read;
          VERIFY(ReadConsoleInput(hStdIn, records, 1, &read));
          size_t keyEvents = 0;
          for(DWORD i = 0; i < read; ++i)
            if(records[i].EventType == KEY_EVENT && records[i].Event.KeyEvent.bKeyDown)
            {
              char_t character = records[i].Event.KeyEvent.uChar.AsciiChar;
              switch(character)
              {
              case _T('\0'):
                switch(records[i].Event.KeyEvent.wVirtualKeyCode)
                {
                case VK_LEFT:
                  promptMoveLeft();
                  break;
                case VK_RIGHT:
                  promptMoveRight();
                  break;
                case VK_DELETE:
                  promptRemoveNext();
                  break;
                case VK_HOME:
                  promptHome();
                  break;
                case VK_END:
                  promptEnd();
                  break;
                }
                break;
              case _T('\t'):
                break;
              case _T('\r'):
                inputComplete = true;
                break;
              case _T('\b'):
                promptRemove();
                break;
              default:
                promptInsert(character);
              }
            }
        }
        break;
      case WAIT_OBJECT_0 + 1:
        {
          // get new output
          DWORD read;
          if(!GetOverlappedResult(hStdOutRead, &overlapped, &read, FALSE))
            continue;

          //
          promptClear();
          restoreCursorPosition();
          restoreTerminalMode();

          // add new output
          DWORD written;
          VERIFY(WriteFile(hOriginalStdOut, stdoutBuffer, read, &written, NULL));
          ASSERT(written == read);
          while(ReadFile(hStdOutRead, stdoutBuffer, sizeof(stdoutBuffer), &read, &overlapped))
          {
            VERIFY(WriteFile(hOriginalStdOut, stdoutBuffer, read, &written, NULL));
            ASSERT(written == read);
          }

          //
          enableTerminalRawMode();
          saveCursorPosition();
          promptWrite();
        }
        break;
      case WAIT_OBJECT_0 + 2:
        break;
      }
    }

#else
    while(!inputComplete)
    {
      if(!bufferedInput.isEmpty())
      {
        handleInput((const char_t*)(const byte_t*)bufferedInput, bufferedInput.size());
        bufferedInput.free();
        if(inputComplete)
          break;
      }

      fd_set fdr;
      FD_ZERO(&fdr);
      FD_SET(stdoutRead, &fdr);
      FD_SET(STDIN_FILENO, &fdr);
      timeval tv = {1000000, 0};
      switch(select(stdoutRead + 1, &fdr, 0, 0, &tv))
      {
      case 0:
        continue;
      case -1:
        ASSERT(false);
        return String();
      }
      if(FD_ISSET(stdoutRead, &fdr))
      {
        promptClear();
        restoreCursorPosition();
        char buffer[4096];
        ssize_t i = read(stdoutRead, buffer, sizeof(buffer));
        VERIFY(i != -1);

        restoreTerminalMode();
        
        VERIFY(write(originalStdout, buffer, i) == i);

        enableTerminalRawMode();
        saveCursorPosition();
        promptWrite();
      }
      if(FD_ISSET(STDIN_FILENO, &fdr))
      {
        char buffer[4096];
        ssize_t i = read(STDIN_FILENO, buffer, sizeof(buffer));
        VERIFY(i != -1);
        handleInput(buffer, i);
      }
    }

#endif
    promptClear();
    restoreCursorPosition();
    restoreTerminalMode();
    return input;
  }

#ifndef _WIN32
  void_t handleInput(const char_t* input, size_t len)
  {
    for(const char_t* end = input + len; input < end; ++input)
      handleInput(*input);
  }

  void_t handleInput(char_t c)
  {
    switch(c)
    {
    case _T('\t'):
      break;
    case _T('\r'):
      inputComplete = true;
      break;
    case _T('\b'):
      promptRemove();
      break;
    default:
      promptInsert(c);

    }
  }
#endif

private:
  bool_t valid;
#ifdef _WIN32
  HANDLE hStdOutRead;
  HANDLE hStdOutWrite;
  HANDLE hOriginalStdOut;
  int originalStdout;
  int newStdout;
  OVERLAPPED overlapped;
  char stdoutBuffer[4096];
  DWORD consoleMode;
#else
  static int originalStdout;
  int stdoutRead;
  int stdoutWrite;
  
  static bool_t originalTermiosValid;
  static termios originalTermios;

  Buffer bufferedInput;
#endif

  size_t stdoutScreenWidth;
  size_t stdoutCursorX;

  String prompt;
  String input;
  bool_t inputComplete;
  size_t caretPos;
};

#ifndef _WIN32
int ConsolePromptPrivate::originalStdout = -1;
bool_t ConsolePromptPrivate::originalTermiosValid = false;
termios ConsolePromptPrivate::originalTermios;
#endif

Console::Prompt::Prompt() : data(new ConsolePromptPrivate) {}

Console::Prompt::~Prompt() {delete data;}

String Console::Prompt::getLine(const String& str) {return data->getLine(str);}
