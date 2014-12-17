
#include <cstdarg>
#include <cstdio>
#ifdef _MSC_VER
#include <tchar.h>
#endif
#ifdef _MSC_VER
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#ifndef __CYGWIN__
#include <sys/eventfd.h>
#endif
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <signal.h>
#include <errno.h>
#endif

#include <nstd/Debug.h>
#include <nstd/Console.h>
#include <nstd/Array.h>
#include <nstd/List.h>
#ifndef _MSC_VER
#include <nstd/Buffer.h>
#include <nstd/Unicode.h>
#include <nstd/Process.h>
#endif

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

#ifdef _MSC_VER
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
#ifdef _MSC_VER
  typedef tchar_t conchar_t;
#else
  typedef uint32_t conchar_t;
#endif

#ifndef _MSC_VER
  static void restoreTermMode()
  {
    if(originalTermiosValid)
    {
      VERIFY(tcsetattr(originalStdout, TCSADRAIN, &originalTermios) == 0);
      originalTermiosValid = false;
    }
  }
  static void handleWinch(int sig)
  {
#ifndef __CYGWIN__
    int eventFd = resizeEventFd;
#else
    int eventFd = resizeEventFdWrite;
#endif
    if(eventFd)
    {
      uint64_t event = 1;
      write(eventFd, &event, sizeof(uint64_t));
    }
  }
#endif

  ConsolePromptPrivate() : valid(false)
  {
#ifdef _MSC_VER
    if(!GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &consoleOutputMode) ||
       !GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &consoleInputMode))
      return; // no tty?
    valid = true;

    // redirect stdout and stderr to pipe
    VERIFY(CreatePipeEx(&hStdOutRead, &hStdOutWrite, NULL, 0, FILE_FLAG_OVERLAPPED, 0));
    originalStdout = _dup(_fileno(stdout));
    originalStderr = _dup(_fileno(stderr));
    newStdout = _open_osfhandle((intptr_t)hStdOutWrite, _O_BINARY);
    ASSERT(newStdout != -1);
    VERIFY(_dup2(newStdout, _fileno(stdout)) == 0);
    VERIFY(_dup2(newStdout, _fileno(stderr)) == 0);
    VERIFY(setvbuf(stdout, NULL, _IONBF, 0) == 0);
    VERIFY(setvbuf(stderr, NULL, _IONBF, 0) == 0);
    SetStdHandle(STD_OUTPUT_HANDLE, (HANDLE)_get_osfhandle(_fileno(stdout)));
    SetStdHandle(STD_ERROR_HANDLE, (HANDLE)_get_osfhandle(_fileno(stderr)));
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

    // enable window events
    VERIFY(SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), consoleInputMode | ENABLE_WINDOW_INPUT));
#else
    if(originalStdout != -1)
      return; // you should not create more than a single instance of this class
    if(!isatty(STDIN_FILENO))
      return;
    valid = true;

#ifdef __CYGWIN__
    originalStdout = open("/dev/null", O_CLOEXEC);
#else
    originalStdout = eventfd(0, EFD_CLOEXEC); // create new file descriptor with O_CLOEXEC.. is there a better way to do this?
#endif
    VERIFY(dup3(STDOUT_FILENO, originalStdout, O_CLOEXEC) != -1); // create copy of STDOUT_FILENO
    VERIFY(dup3(STDERR_FILENO, originalStderr, O_CLOEXEC) != -1); // create copy of STDOUT_FILENO
    int pipes[2];
    VERIFY(pipe2(pipes, O_CLOEXEC) == 0);
    stdoutRead = pipes[0];
    stdoutWrite = pipes[1];
    VERIFY(dup2(stdoutWrite, STDOUT_FILENO) != -1);
    VERIFY(dup2(stdoutWrite, STDERR_FILENO) != -1);
    VERIFY(setvbuf(stdout, NULL, _IONBF, 0) == 0);
    VERIFY(setvbuf(stderr, NULL, _IONBF, 0) == 0);

    // save term mode
    if(!originalTermiosValid)
    {
      VERIFY(tcgetattr(originalStdout, &originalTermios) == 0);
      originalTermiosValid = true;
      atexit(restoreTermMode);

      // get raw mode
      rawMode = originalTermios;
      cfmakeraw(&rawMode);
      rawMode.c_lflag |= ISIG;
      rawMode.c_cc[VMIN] = 1;
      rawMode.c_cc[VTIME] = 0;
      rawMode.c_oflag |= OPOST; // workaround for cygwin, TCSADRAIN does not seem to work reliably

      // disable input character echo
      noEchoMode = rawMode;
      noEchoMode.c_oflag |= OPOST;
      VERIFY(tcsetattr(originalStdout, TCSADRAIN, &noEchoMode) == 0);
    }

    // install console window resize signal handler
    if(!resizeEventFd)
    {
#ifdef __CYGWIN__
      int pipefd[2];
      VERIFY(pipe2(pipefd, O_CLOEXEC) == 0);
      resizeEventFd = pipefd[0];
      resizeEventFdWrite = pipefd[1];
#else
      resizeEventFd = eventfd(0, EFD_CLOEXEC);
#endif
      signal(SIGWINCH, handleWinch);
    }

    // utf8 console?
    utf8 = Process::getEnvironmentVariable("LANG").endsWith(".UTF-8"); // todo: endsWithNoCase?
#endif

    // get screen width
    stdoutScreenWidth = getScreenWidth();
  }

  ~ConsolePromptPrivate()
  {
    if(!valid)
      return;
    redirectPendingData();
    //hier muss ich alles von stderr und stdout lesen und nach was auch immer schreiben
    //  vermutlich muss ich beim lesen stderr und stdout in getLine auch unter zwischen diesen beiden unterscheiden, damit ich sie an den richtigen original stream senden kann
#ifdef _MSC_VER
    CancelIo(hStdOutRead);
    CloseHandle(overlapped.hEvent);
    _dup2(originalStdout, _fileno(stdout));
    _dup2(originalStderr, _fileno(stderr));
    SetStdHandle(STD_OUTPUT_HANDLE, (HANDLE)_get_osfhandle(_fileno(stdout)));
    SetStdHandle(STD_ERROR_HANDLE, (HANDLE)_get_osfhandle(_fileno(stderr)));
    _close(originalStdout); // this should close hOriginalStdOut
    _close(originalStderr);
    _close(newStdout); // this should close hStdOutWrite
    CloseHandle(hStdOutRead);
    VERIFY(SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), consoleInputMode));
#else
    if(resizeEventFd)
    {
      signal(SIGWINCH, SIG_IGN);
      close(resizeEventFd);
#ifdef __CYGWIN__
      close(resizeEventFdWrite);
      resizeEventFdWrite = 0;
#endif
      resizeEventFd = 0;
    }
    restoreTermMode();
    originalTermiosValid = false;
    VERIFY(dup2(originalStdout, STDOUT_FILENO) != -1);
    VERIFY(dup2(originalStderr, STDERR_FILENO) != -1);
    close(originalStdout);
    close(originalStderr);
    close(stdoutRead);
    close(stdoutWrite);
    originalStdout = -1;
#endif
  }

  size_t getScreenWidth()
  {
#ifdef _MSC_VER
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    VERIFY(GetConsoleScreenBufferInfo(hOriginalStdOut, &csbi));
    return csbi.dwSize.X;
#else
    winsize ws;
    if(ioctl(originalStdout, TIOCGWINSZ, &ws) == 0)
      if(ws.ws_col)
        return ws.ws_col;
    size_t x, y;
    getCursorPosition(x, y);
    write(originalStdout, "\x1b[999C", 6);
    size_t newX, newY;
    getCursorPosition(newX, newY);
    if(newX > x)
    {
      String moveCmd;
      moveCmd.printf("\x1b[%dD",newX - x);
      write(originalStdout, (const char_t*)moveCmd, moveCmd.length());
    }
    return newX + 1;
#endif
  }

  void_t getCursorPosition(size_t& x, size_t& y)
  {
#ifdef _MSC_VER
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    VERIFY(GetConsoleScreenBufferInfo(hOriginalStdOut, &csbi));
    x = csbi.dwCursorPosition.X;
    y = csbi.dwCursorPosition.Y;
#else
    VERIFY(write(originalStdout, "\x1b[6n", 4) == 4);
    char_t buffer[64];
    VERIFY(readNextUnbufferedEscapedSequence(buffer, '[', 'R') > 1);
    int ix, iy;
    VERIFY(sscanf(buffer + 2, "%d;%d", &iy, &ix) == 2);
    x = ix - 1;
    y = iy - 1;
#endif
  }

  void_t moveCursorPosition(size_t from, ssize_t x)
  {
#ifdef _MSC_VER
    size_t to = from + x;
    size_t oldY = from / stdoutScreenWidth;
    size_t newY = to / stdoutScreenWidth;
    size_t newX = to % stdoutScreenWidth;
    size_t cursorX, cursorY;
    getCursorPosition(cursorX, cursorY);
    COORD pos = {(SHORT)newX, (SHORT)(cursorY + ((ssize_t)newY - (ssize_t)oldY))};
    VERIFY(SetConsoleCursorPosition(hOriginalStdOut, pos)); // this resets the caret blink timer
#else
    size_t to = from + x;
    size_t oldY = from / stdoutScreenWidth;
    size_t oldX = from % stdoutScreenWidth;
    size_t newY = to / stdoutScreenWidth;
    size_t newX = to % stdoutScreenWidth;
    String moveCmd;
    if(newY < oldY)
      moveCmd.printf("\x1b[%dA", (int)(oldY - newY));
    else if(newY > oldY)
      moveCmd.printf("\x1b[%dB", (int)(newY - oldY));
    if(newX < oldX)
    {
      String add;
      add.printf("\x1b[%dD", (int)(oldX - newX));
      moveCmd += add;
    }
    else if(newX > oldX)
    {
      String add;
      add.printf("\x1b[%dC", (int)(newX - oldX));
      moveCmd += add;
    }
    if(!moveCmd.isEmpty())
      writeConsole(moveCmd, moveCmd.length());
#endif
  }

  void_t writeConsole(const tchar_t* data, size_t len)
  {
#ifdef _MSC_VER
    DWORD written;
    VERIFY(WriteConsole(hOriginalStdOut, data, (DWORD)len, &written, NULL));
    ASSERT(written == (DWORD)len);
#else
    bufferedOutput.append((const byte_t*)data, len);
#endif
  }

#ifndef _MSC_VER
  void_t flushConsole()
  {
    if(bufferedOutput.isEmpty())
      return;
    VERIFY(write(originalStdout, bufferedOutput, bufferedOutput.size()) == (ssize_t)bufferedOutput.size());
    bufferedOutput.free();
  }
#endif

  void_t promptWrite(size_t offset = 0, const String& clearStr = String())
  {
    offset += offset / stdoutScreenWidth * 2;
    Array<conchar_t> buffer(prompt.size() + input.size() + clearStr.length());
    buffer.append(prompt);
    buffer.append(input);
    ASSERT(clearStr.isEmpty() || clearStr.length() == 1);
    if(!clearStr.isEmpty())
      buffer.append(*(const tchar_t*)clearStr);
    Array<conchar_t> wrappedBuffer(buffer.size() + buffer.size() / stdoutScreenWidth * 2 + 2);
    for(size_t i = 0, len = buffer.size(); i < len; i += stdoutScreenWidth)
    {
      size_t lineEnd = len - i;
      if(lineEnd > stdoutScreenWidth)
        lineEnd = stdoutScreenWidth;
      wrappedBuffer.append((const conchar_t*)buffer + i, lineEnd);
      if(lineEnd == stdoutScreenWidth)
      {
        wrappedBuffer.append(_T('\r'));
        wrappedBuffer.append(_T('\n'));
      }
    }
#ifdef _MSC_VER
    writeConsole((const conchar_t*)wrappedBuffer + offset, wrappedBuffer.size() - offset);
#else
    String dataToWrite((wrappedBuffer.size() - offset) * sizeof(uint32_t) + 10);
    dataToWrite.append("\x1b[?7l");
    VERIFY(Unicode::append((const conchar_t*)wrappedBuffer + offset, wrappedBuffer.size() - offset, dataToWrite));
    dataToWrite.append("\x1b[?7h");
    writeConsole(dataToWrite, dataToWrite.length());
#endif
    if(caretPos < input.size() + clearStr.length())
      moveCursorPosition(prompt.size() + input.size() + clearStr.length(), -(ssize_t)(input.size() + clearStr.length() - caretPos));
#ifdef _MSC_VER
    else // enforce cursor blink timer reset
      moveCursorPosition(prompt.size() + input.size() + clearStr.length(), 0);
#endif
  }

  void_t promptInsert(conchar_t character)
  {
    if(caretPos != input.size())
    {
      Array<conchar_t> newInput(input.size() + 1);
      newInput.append(input,  caretPos);
      newInput.append(character);
      newInput.append((conchar_t*)input + caretPos, input.size() - caretPos);
      input.swap(newInput);
    }
    else
      input.append(character);
    size_t oldCaretPos = caretPos;
    ++caretPos;
    promptWrite(prompt.size() + oldCaretPos);
  }

  void_t promptRemove()
  {
    if(!input.isEmpty() && caretPos > 0)
    {
      if(caretPos != input.size())
      {
        Array<conchar_t> newInput(input.size() - 1);
        newInput.append(input,  caretPos -1);
        newInput.append((conchar_t*)input + caretPos, input.size() - caretPos);
        input.swap(newInput);
      }
      else
        input.resize(input.size() - 1);
      promptMoveLeft();
      promptWrite(prompt.size() + caretPos, _T(" "));
    }
  }

  void_t promptRemoveNext()
  {
    if(caretPos < input.size())
    {
      if(caretPos != input.size() - 1)
      {
        Array<conchar_t> newInput(input.size() - 1);
        newInput.append(input,  caretPos);
        newInput.append((conchar_t*)input + caretPos + 1, input.size() - (caretPos + 1));
        input.swap(newInput);
      }
      else
        input.resize(input.size() - 1);
      promptWrite(prompt.size() + caretPos, _T(" "));
    }
  }

  void_t promptMoveLeft()
  {
    if(caretPos > 0)
    {
      moveCursorPosition(prompt.size() + caretPos, -1);
      --caretPos;
    }
  }

  void_t promptMoveHome()
  {
    if(caretPos > 0)
    {
      moveCursorPosition(prompt.size() + caretPos, -(ssize_t)caretPos);
      caretPos = 0;
    }
  }

  void_t promptMoveRight()
  {
    if(caretPos < input.size())
    {
      moveCursorPosition(prompt.size() + caretPos, 1);
      ++caretPos;
    }
  }

  void_t promptMoveEnd()
  {
    if(caretPos < input.size())
    {
      moveCursorPosition(prompt.size() + caretPos, input.size() - caretPos);
      caretPos = input.size();
    }
  }

  void_t promptHistoryUp()
  {
    if(historyPos == history.begin())
      return;
    String currentLine;
    concharArrayToString(input, input.size(), currentLine);
    if(historyPos == history.end())
    {
      --historyPos;
      history.append(currentLine);
      historyRemoveLast = true;
    }
    else
    {
      *historyPos = currentLine;
      --historyPos;
    }
    promptClear();
    stringToConcharArray(*historyPos, input);
    caretPos = input.size();
    promptWrite();
  }

  void_t promptHistoryDown()
  {
    if(historyPos == history.end())
      return;
    String currentLine;
    concharArrayToString(input, input.size(), currentLine);
    *historyPos = currentLine;
    ++historyPos;
    promptClear();
    stringToConcharArray(*historyPos, input);
    caretPos = input.size();
    promptWrite();
    List<String>::Iterator next = historyPos;
    ++next;
    if(next == history.end())
    {
      history.removeBack();
      historyPos = history.end();
      historyRemoveLast = false;
    }
  }

  void_t promptClear()
  {
    size_t bufferLen = prompt.size() + input.size();
    size_t additionalLines = bufferLen / stdoutScreenWidth;
    if(additionalLines)
    {
      moveCursorPosition(prompt.size() + caretPos, -(ssize_t)(caretPos + prompt.size()));
      String clearLine(stdoutScreenWidth + 2);
      for(size_t i = 0; i < stdoutScreenWidth; ++i)
        clearLine.append(_T(' '));
      clearLine.append(_T("\n\r"));
      String clearCmd(bufferLen + additionalLines * 2
#ifndef _MSC_VER
        + 10
#endif
      );
#ifndef _MSC_VER
      clearCmd.append("\x1b[?7l");
#endif
      for(size_t i = 0; i < additionalLines; ++i)
        clearCmd.append(clearLine);
      for(size_t i = 0, count = bufferLen - additionalLines * stdoutScreenWidth; i < count; ++i)
        clearCmd.append(_T(' '));
#ifndef _MSC_VER
      clearCmd.append("\x1b[?7h");
#endif
      writeConsole(clearCmd, clearCmd.length());
      moveCursorPosition(prompt.size() + input.size(), -(ssize_t)bufferLen);
    }
    else
    {
      String clearCmd(2 + bufferLen
#ifndef _MSC_VER
        + 10
#endif
      );
#ifndef _MSC_VER
      clearCmd.append("\x1b[?7l");
#endif
      clearCmd.append(_T('\r'));
      for(size_t i = 0, count = bufferLen; i < count; ++i)
        clearCmd.append(_T(' '));
      clearCmd.append(_T('\r'));
#ifndef _MSC_VER
      clearCmd.append("\x1b[?7h");
#endif
      writeConsole(clearCmd, clearCmd.length());
    }
  }

  void_t saveCursorPosition()
  {
    size_t x, y;
    getCursorPosition(x, y);
    stdoutCursorX = x;
    if(stdoutCursorX)
      writeConsole(_T("\r\n"), 2);
  }

  void_t restoreCursorPosition()
  {
    if(stdoutCursorX)
    {
#ifdef _MSC_VER
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
#ifdef _MSC_VER
    VERIFY(SetConsoleMode(hOriginalStdOut, consoleOutputMode));
#else
    VERIFY(tcsetattr(originalStdout, TCSADRAIN, &noEchoMode) == 0);
#endif
  }

  void_t enableTerminalRawMode()
  {
#ifdef _MSC_VER
    VERIFY(SetConsoleMode(hOriginalStdOut, ENABLE_PROCESSED_OUTPUT));
#else
    VERIFY(tcsetattr(originalStdout, TCSADRAIN, &rawMode) == 0);
#endif
  }

  String getLine(const String& prompt)
  {
    if(!valid)
    {
      // todo: print std and read line from stdin 
      return String();
    }

    // write pending stdout data
    redirectPendingData();

    //
    enableTerminalRawMode();
    saveCursorPosition();

    // initialize and write prompt
    stringToConcharArray(prompt, this->prompt);
    historyPos = history.end();
    historyRemoveLast = false;
    input.clear();
    inputComplete = false;
    caretPos = 0;
    promptWrite();

#ifdef _MSC_VER
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
            if(records[i].EventType == WINDOW_BUFFER_SIZE_EVENT)
            {
              promptClear();
              stdoutScreenWidth = records[i].Event.WindowBufferSizeEvent.dwSize.X;
              promptWrite();
            }
            else if(records[i].EventType == KEY_EVENT && records[i].Event.KeyEvent.bKeyDown)
            {
#ifdef _UNICODE
              tchar_t character = records[i].Event.KeyEvent.uChar.UnicodeChar;
#else
              tchar_t character = records[i].Event.KeyEvent.uChar.AsciiChar;
#endif
              switch(character)
              {
              case _T('\0'):
                switch(records[i].Event.KeyEvent.wVirtualKeyCode)
                {
                case VK_UP:
                  promptHistoryUp();
                  break;
                case VK_DOWN:
                  promptHistoryDown();
                  break;
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
                  promptMoveHome();
                  break;
                case VK_END:
                  promptMoveEnd();
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
          VERIFY(WriteConsole(hOriginalStdOut, stdoutBuffer, read / sizeof(tchar_t), &written, NULL));
          ASSERT(written == read / sizeof(tchar_t));
          while(ReadFile(hStdOutRead, stdoutBuffer, sizeof(stdoutBuffer), &read, &overlapped))
          {
            VERIFY(WriteConsole(hOriginalStdOut, stdoutBuffer, read / sizeof(tchar_t), &written, NULL));
            ASSERT(written == read / sizeof(tchar_t));
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
    for(;;)
    {
      while(!bufferedInput.isEmpty())
      {
        char_t buffer[64];
        size_t len = readChar(buffer);
        if(*buffer == '\x1b')
          handleEscapedInput(buffer, len);
        else
          handleInput(buffer, len);
        if(inputComplete)
          break;
      }
      if(inputComplete)
        break;

      fd_set fdr;
      FD_ZERO(&fdr);
      FD_SET(stdoutRead, &fdr);
      FD_SET(STDIN_FILENO, &fdr);
      FD_SET(resizeEventFd, &fdr);
      timeval tv = {1000000, 0};
      flushConsole();
      switch(select((stdoutRead > resizeEventFd ? stdoutRead : resizeEventFd) + 1, &fdr, 0, 0, &tv))
      {
      case 0:
        continue;
      case -1:
        if(errno == EINTR)
          continue;
        ASSERT(false);
        return String();
      }
      if(FD_ISSET(stdoutRead, &fdr))
      {
        promptClear();
        restoreCursorPosition();
        restoreTerminalMode();

        char buffer[4096];
        ssize_t i = read(stdoutRead, buffer, sizeof(buffer));
        VERIFY(i != -1);
        writeConsole(buffer, i);

        flushConsole();
        enableTerminalRawMode();
        saveCursorPosition();
        promptWrite();
      }
      if(FD_ISSET(STDIN_FILENO, &fdr))
      {
        char buffer[64];
        size_t len = readChar(buffer);
        if(*buffer == '\x1b')
          handleEscapedInput(buffer, len);
        else
          handleInput(buffer, len);
      }
      if(FD_ISSET(resizeEventFd, &fdr))
      {
        uint64_t buffer;
        read(resizeEventFd, &buffer, sizeof(uint64_t));
        promptClear();
        stdoutScreenWidth = getScreenWidth();
        promptWrite();
      }
    }

#endif
    promptClear();
    restoreCursorPosition();
#ifndef _MSC_VER
    flushConsole();
#endif
    restoreTerminalMode();
    String result;
    concharArrayToString(input, input.size(), result);
    if(historyRemoveLast)
      history.removeBack();
    if(!result.isEmpty())
      history.append(result);
    return result;
  }

  void_t redirectPendingData()
  {
#ifdef _MSC_VER
    {
      DWORD read;
      while(GetOverlappedResult(hStdOutRead, &overlapped, &read, FALSE))
      {
         DWORD written;
         VERIFY(WriteConsole(hOriginalStdOut, stdoutBuffer, read / sizeof(tchar_t), &written, NULL));
         ASSERT(written == read / sizeof(tchar_t));
         while(ReadFile(hStdOutRead, stdoutBuffer, sizeof(stdoutBuffer), &read, &overlapped))
         {
           VERIFY(WriteConsole(hOriginalStdOut, stdoutBuffer, read / sizeof(tchar_t), &written, NULL));
           ASSERT(written == read / sizeof(tchar_t));
         }
      }
    }
#else
    for(;;)
    {
      fd_set fdr;
      FD_ZERO(&fdr);
      FD_SET(stdoutRead, &fdr);
      timeval tv = {0, 0};
      switch(select(stdoutRead + 1, &fdr, 0, 0, &tv))
      {
      case 1:
        {
          char buffer[4096];
          ssize_t i = read(stdoutRead, buffer, sizeof(buffer));
          VERIFY(i != -1);
          VERIFY(write(originalStdout, buffer, i) == i);
        }
        continue;
      case 0:
        break;
      default:
        ASSERT(false);
        return;
      }
      break;
    }
#endif
  }

#ifndef _MSC_VER
  size_t readChar(char_t* buffer)
  {
    if(utf8)
      return readBufferedUtf8Char(buffer);
    return readBufferedChar(buffer);
  }

  size_t readNextUnbufferedEscapedSequence(char_t* buffer, char_t firstChar, char_t lastChar)
  {
    for(char_t ch;;)
    {
      VERIFY(read(STDIN_FILENO, &ch, 1) == 1);
      if(ch == '\x1b')
      {
        *buffer = ch;
        size_t len = 1 + readUnbufferedEscapedSequence(buffer, buffer + 1);
        char_t sequenceType = buffer[len - 1];
        if(sequenceType != lastChar || buffer[1] != firstChar)
        {
          bufferedInput.append((byte_t*)buffer, len);
          continue;
        }
        return len;
      }
      else
        bufferedInput.append((byte_t*)&ch, 1);
    }
  }

  size_t readBufferedUtf8Char(char_t* buffer)
  {
    if(bufferedInput.isEmpty())
    {
      char_t ch;
      VERIFY(read(STDIN_FILENO, &ch, 1) == 1);
      if(ch == '\x1b')
      {
        *buffer = '\x1b';
        return 1 + readUnbufferedEscapedSequence(buffer, buffer + 1);
      }
      *buffer = ch;
      return readUnbufferedUtf8Char(buffer, buffer + 1, Unicode::length(ch));
    }
    *buffer = *(const char_t*)(const byte_t*)bufferedInput;
    bufferedInput.removeFront(1);
    if(*buffer == '\x1b')
      return 1 + readBufferedEscapedSequence(buffer, buffer + 1);
    char_t* start = buffer;
    size_t len = Unicode::length(*(buffer++));
    while((size_t)(buffer - start) < len)
    {
      if(bufferedInput.isEmpty())
        return readUnbufferedUtf8Char(start, buffer, len);
      *buffer = *(const char_t*)(const byte_t*)bufferedInput;
      bufferedInput.removeFront(1);
      if(*buffer == '\x1b')
      {
        Buffer incompleteUtf8((const byte_t*)start, buffer - start);
        *start = '\x1b';
        size_t result = 1 + readBufferedEscapedSequence(start, start + 1);
        bufferedInput.prepend(incompleteUtf8);
        return result;
      }
      ++buffer;
    }
    *buffer = '\0';
    return len;
  }

  size_t readUnbufferedUtf8Char(char_t* start,  char_t* buffer, size_t len)
  {
    while((size_t)(buffer - start) < len)
    {
      VERIFY(read(STDIN_FILENO, buffer, 1) == 1);
      ++buffer;
    }
    *buffer = '\0';
    return len;
  }

  size_t readBufferedChar(char_t* buffer)
  {
    if(bufferedInput.isEmpty())
      return readUnbufferedChar(buffer);
    *buffer = *(const char_t*)(const byte_t*)bufferedInput;
    bufferedInput.removeFront(1);
    if(*buffer == '\x1b')
      return 1 + readBufferedEscapedSequence(buffer, buffer + 1);
    buffer[1] = '\0';
    return 1;
  }

  size_t readUnbufferedChar(char_t* buffer)
  {
    VERIFY(read(STDIN_FILENO, buffer, 1) == 1);
    if(*buffer == '\x1b')
      return 1 + readUnbufferedEscapedSequence(buffer, buffer + 1);
    buffer[1] = '\0';
    return 1;
  }

  static bool_t isSeqAttributeChar(char_t ch) {return isdigit(ch) || ch == ';' || ch == '?' || ch == '[';}

  size_t readBufferedEscapedSequence(const char_t* seqStart, char_t* buffer)
  {
    for(char_t* start = buffer;;)
    {
      if(bufferedInput.isEmpty())
        return buffer - start + readUnbufferedEscapedSequence(seqStart, buffer);
      *buffer = *(const char_t*)(const byte_t*)bufferedInput;
      bufferedInput.removeFront(1);
      if(seqStart[1] != '[' || !isSeqAttributeChar(*buffer))
      {
        ++buffer;
        *buffer = '\0';
        return buffer - start;
      }
      ++buffer;
    }
  }

  size_t readUnbufferedEscapedSequence(const char_t* seqStart, char_t* buffer)
  {
    for(char_t* start = buffer, ch;;)
    {
      VERIFY(read(STDIN_FILENO, &ch, 1) == 1);
      *(buffer++) = ch;
      if(seqStart[1] != '[' || !isSeqAttributeChar(ch))
      {
        *buffer = '\0';
        return buffer - start;
      }
    }
  }

  void_t handleEscapedInput(char_t* input, size_t len)
  {
    if(input[1] == '[')
      switch(input[2])
      {
      case 'A': // up
        promptHistoryUp();
        break;
      case 'B': // down
        promptHistoryDown();
        break;
      case 'C': // right
        promptMoveRight();
        break;
      case 'D': // left
        promptMoveLeft();
        break;
      case 'H': // home
        promptMoveHome();
        break;
      case 'F': // end
        promptMoveEnd();
        break;
      case '3':
        if(input[3] == '~') // del
          promptRemoveNext();
        break;
      }
  }

  void_t handleInput(char_t* input, size_t len)
  {
    uint32_t ch = utf8 ? Unicode::fromString(input, len) : *(uchar_t*)input;
    switch(ch)
    {
    case _T('\t'):
      break;
    case _T('\r'):
      inputComplete = true;
      break;
    case '\b': // backspace
    case 127: // del
      promptRemove();
      break;
    default:
      promptInsert(ch);
    }
  }
#endif

  void_t concharArrayToString(const conchar_t* data, size_t len, String& result)
  {
    result.clear();
#ifdef _MSC_VER
    result.append(data, len);
#else
    if(utf8)
    {
      result.reserve(len * sizeof(uint32_t));
      Unicode::append(data, len, result);
    }
    else
    {
      result.reserve(len);
      for(const conchar_t* i = data, * end = data + len; i < end; ++i)
        result.append((const tchar_t&)*i);
    }
#endif
  }

  void_t stringToConcharArray(const String& data, Array<conchar_t>& result)
  {
    result.clear();
    result.reserve(data.length());
#ifdef _MSC_VER
    for(const tchar_t* i = data, * end = i + data.length(); i < end; ++i)
      result.append(*i);
#else
    if(utf8)
    {
      for(const tchar_t* i = data, * end = i + data.length(); i < end;)
      {
        size_t len = Unicode::length(*i);
        if(len == 0 || (size_t)(end - i) < len)
        {
          ++i;
          continue;
        }
        result.append(Unicode::fromString(i, len));
        i += len;
      }
    }
    else
      for(const tchar_t* i = data, * end = i + data.length(); i < end; ++i)
        result.append((const uchar_t&)*i);
#endif
  }

private:
  bool_t valid;
#ifdef _MSC_VER
  HANDLE hStdOutRead;
  HANDLE hStdOutWrite;
  HANDLE hOriginalStdOut;
  int originalStdout;
  int originalStderr;
  int newStdout;
  OVERLAPPED overlapped;
  char stdoutBuffer[4096];
  DWORD consoleOutputMode;
  DWORD consoleInputMode;
#else
  bool_t utf8;
  static int originalStdout;
  int originalStderr;
  int stdoutRead;
  int stdoutWrite;
  
  static bool_t originalTermiosValid;
  static termios originalTermios;
  termios rawMode;
  termios noEchoMode;
  static int resizeEventFd;
#ifdef __CYGWIN__
  static int resizeEventFdWrite;
#endif

  Buffer bufferedInput;
  Buffer bufferedOutput;
#endif

  size_t stdoutScreenWidth;
  size_t stdoutCursorX;

  Array<conchar_t> prompt;
  Array<conchar_t> input;
  bool_t inputComplete;
  size_t caretPos;

  List<String> history;
  List<String>::Iterator historyPos;
  bool_t historyRemoveLast;
};

#ifndef _MSC_VER
int ConsolePromptPrivate::originalStdout = -1;
bool_t ConsolePromptPrivate::originalTermiosValid = false;
termios ConsolePromptPrivate::originalTermios;
int ConsolePromptPrivate::resizeEventFd = 0;
#ifdef __CYGWIN__
int ConsolePromptPrivate::resizeEventFdWrite = 0;
#endif
#endif

Console::Prompt::Prompt() : data(new ConsolePromptPrivate) {}

Console::Prompt::~Prompt() {delete data;}

String Console::Prompt::getLine(const String& str) {return data->getLine(str);}
