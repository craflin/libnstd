
#include <cstdarg>
#include <cstdio>
#ifdef _MSC_VER
#include <tchar.h>
#endif
#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
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
#include <cassert>
#include <signal.h>
#include <errno.h>
#endif

#include <nstd/Debug.hpp>
#include <nstd/Console.hpp>
#include <nstd/Array.hpp>
#include <nstd/List.hpp>
#ifndef _MSC_VER
#include <nstd/Buffer.hpp>
#include <nstd/Unicode.hpp>
#include <nstd/Process.hpp>
#endif

int Console::print(const tchar* str)
{
#ifdef _MSC_VER
  return _fputts(str, stdout);
#else
  return fputs(str, stdout);
#endif
}

int Console::printf(const tchar* format, ...)
{
  va_list ap;
  va_start(ap, format);
#ifdef _UNICODE
  int result = vwprintf(format, ap);
#else
  int result = vprintf(format, ap);
#endif
  va_end(ap);
  return result;
}

int Console::error(const tchar* str)
{
#ifdef _MSC_VER
  return _fputts(str, stderr);
#else
  return fputs(str, stderr);
#endif
}

int Console::errorf(const tchar* format, ...)
{
  va_list ap;
  va_start(ap, format);
#ifdef _UNICODE
  int result = vfwprintf(stderr, format, ap);
#else
  int result = vfprintf(stderr, format, ap);
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
  HANDLE hRead = CreateNamedPipe((const tchar*)name, PIPE_ACCESS_INBOUND | dwReadMode, PIPE_TYPE_BYTE | PIPE_WAIT, 1, nSize, nSize, 120 * 1000, lpSecurityAttributes);
  if(!hRead)
    return FALSE;
  HANDLE hWrite = CreateFile((const tchar*)name, GENERIC_WRITE, 0, lpSecurityAttributes, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | dwWriteMode, NULL);
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

class Console::Prompt::Private
{
public:
#ifdef _MSC_VER
  typedef tchar conchar;
#else
  typedef uint32 conchar;
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
      uint64 event = 1;
      if(write(eventFd, &event, sizeof(uint64)) != sizeof(uint64))
        return;
    }
  }
#endif

  Private() : valid(false)
  {
#ifdef _MSC_VER
    if(!GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &consoleOutputMode) ||
       !GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &consoleInputMode))
      return; // no tty?
    valid = true;

    // redirect stdout and stderr to pipe
    VERIFY(CreatePipeEx(&hStdOutRead, &hStdOutWrite, NULL, 0, FILE_FLAG_OVERLAPPED, 0));
    VERIFY(CreatePipeEx(&hStdErrRead, &hStdErrWrite, NULL, 0, FILE_FLAG_OVERLAPPED, 0));
    originalStdout = _dup(_fileno(stdout));
    originalStderr = _dup(_fileno(stderr));
    newStdout = _open_osfhandle((intptr_t)hStdOutWrite, _O_BINARY);
    ASSERT(newStdout != -1);
    newStderr = _open_osfhandle((intptr_t)hStdErrWrite, _O_BINARY);
    VERIFY(_dup2(newStdout, _fileno(stdout)) == 0);
    VERIFY(_dup2(newStderr, _fileno(stderr)) == 0);
    VERIFY(setvbuf(stdout, NULL, _IONBF, 0) == 0);
    VERIFY(setvbuf(stderr, NULL, _IONBF, 0) == 0);
    SetStdHandle(STD_OUTPUT_HANDLE, (HANDLE)_get_osfhandle(_fileno(stdout)));
    SetStdHandle(STD_ERROR_HANDLE, (HANDLE)_get_osfhandle(_fileno(stderr)));
    hOriginalStdOut = (HANDLE)_get_osfhandle(originalStdout);
    ASSERT(hOriginalStdOut != INVALID_HANDLE_VALUE);
    hOriginalStdErr = (HANDLE)_get_osfhandle(originalStderr);
    ASSERT(hOriginalStdErr != INVALID_HANDLE_VALUE);

    // initialize overlapped reading of hStdOutRead
    ZeroMemory(&outOverlapped, sizeof(outOverlapped));
    ZeroMemory(&errOverlapped, sizeof(errOverlapped));
    VERIFY(outOverlapped.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL));
    VERIFY(errOverlapped.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL));
    DWORD read;
    while(ReadFile(hStdOutRead, stdoutBuffer, sizeof(stdoutBuffer), &read, &outOverlapped))
    {
      DWORD written;
      VERIFY(WriteConsole(hOriginalStdOut, stdoutBuffer, read / sizeof(tchar), &written, NULL));
      ASSERT(written == read / sizeof(tchar));
    }
    while(ReadFile(hStdErrRead, stderrBuffer, sizeof(stderrBuffer), &read, &errOverlapped))
    {
      DWORD written;
      VERIFY(WriteFile(hOriginalStdErr, stderrBuffer, read / sizeof(tchar), &written, NULL));
      ASSERT(written == read / sizeof(tchar));
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
    originalStderr = open("/dev/null", O_CLOEXEC);
#else
    originalStdout = eventfd(0, EFD_CLOEXEC); // create new file descriptor with O_CLOEXEC.. is there a better way to do this?
    originalStderr = eventfd(0, EFD_CLOEXEC); // create new file descriptor with O_CLOEXEC.. is there a better way to do this?
#endif
    VERIFY(dup3(STDOUT_FILENO, originalStdout, O_CLOEXEC) != -1); // create copy of STDOUT_FILENO
    VERIFY(dup3(STDERR_FILENO, originalStderr, O_CLOEXEC) != -1); // create copy of STDERR_FILENO
    int stdoutPipes[2];
    int stderrPipes[2];
    VERIFY(pipe2(stdoutPipes, O_CLOEXEC) == 0);
    VERIFY(pipe2(stderrPipes, O_CLOEXEC) == 0);
    stdoutRead = stdoutPipes[0];
    stdoutWrite = stdoutPipes[1];
    stderrRead = stderrPipes[0];
    stderrWrite = stderrPipes[1];
    VERIFY(dup2(stdoutWrite, STDOUT_FILENO) != -1);
    VERIFY(dup2(stderrWrite, STDERR_FILENO) != -1);
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

  ~Private()
  {
    if(!valid)
      return;
    redirectPendingData();
#ifdef _MSC_VER
    CancelIo(hStdOutRead);
    CancelIo(hStdErrRead);
    CloseHandle(outOverlapped.hEvent);
    CloseHandle(errOverlapped.hEvent);
    _dup2(originalStdout, _fileno(stdout));
    _dup2(originalStderr, _fileno(stderr));
    SetStdHandle(STD_OUTPUT_HANDLE, (HANDLE)_get_osfhandle(_fileno(stdout)));
    SetStdHandle(STD_ERROR_HANDLE, (HANDLE)_get_osfhandle(_fileno(stderr)));
    _close(originalStdout); // this should close hOriginalStdOut
    _close(originalStderr); // this should close hOriginalStdErr
    _close(newStdout); // this should close hStdOutWrite
    _close(newStderr); // this should close hStdOutWrite
    CloseHandle(hStdOutRead);
    CloseHandle(hStdErrRead);
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
    close(stderrRead);
    close(stderrWrite);
    originalStdout = -1;
#endif
  }

  usize getScreenWidth()
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
    usize x, y;
    getCursorPosition(x, y);
    if(write(originalStdout, "\x1b[999C", 6) != 6)
      return 80;
    usize newX, newY;
    getCursorPosition(newX, newY);
    if(newX > x)
    {
      String moveCmd;
      moveCmd.printf("\x1b[%dD", (int)(newX - x));
      if(write(originalStdout, (const char*)moveCmd, moveCmd.length()) != (ssize)moveCmd.length())
        return 80;
    }
    return newX + 1;
#endif
  }

  void getCursorPosition(usize& x, usize& y)
  {
#ifdef _MSC_VER
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    VERIFY(GetConsoleScreenBufferInfo(hOriginalStdOut, &csbi));
    x = csbi.dwCursorPosition.X;
    y = csbi.dwCursorPosition.Y;
#else
    VERIFY(write(originalStdout, "\x1b[6n", 4) == 4);
    char buffer[64];
    VERIFY(readNextUnbufferedEscapedSequence(buffer, '[', 'R') > 1);
    int ix, iy;
    VERIFY(sscanf(buffer + 2, "%d;%d", &iy, &ix) == 2);
    x = ix - 1;
    y = iy - 1;
#endif
  }

  void moveCursorPosition(usize from, ssize x)
  {
#ifdef _MSC_VER
    usize to = from + x;
    usize oldY = from / stdoutScreenWidth;
    usize newY = to / stdoutScreenWidth;
    usize newX = to % stdoutScreenWidth;
    usize cursorX, cursorY;
    getCursorPosition(cursorX, cursorY);
    COORD pos = {(SHORT)newX, (SHORT)(cursorY + ((ssize)newY - (ssize)oldY))};
    VERIFY(SetConsoleCursorPosition(hOriginalStdOut, pos)); // this resets the caret blink timer
#else
    usize to = from + x;
    assert(stdoutScreenWidth != 0);
    usize oldY = from / stdoutScreenWidth;
    usize oldX = from % stdoutScreenWidth;
    usize newY = to / stdoutScreenWidth;
    usize newX = to % stdoutScreenWidth;
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

  void writeConsole(const tchar* data, usize len)
  {
#ifdef _MSC_VER
    DWORD written;
    VERIFY(WriteConsole(hOriginalStdOut, data, (DWORD)len, &written, NULL));
    ASSERT(written == (DWORD)len);
#else
    bufferedOutput.append((const byte*)data, len);
#endif
  }

#ifndef _MSC_VER
  void flushConsole()
  {
    if(bufferedOutput.isEmpty())
      return;
    VERIFY(write(originalStdout, bufferedOutput, bufferedOutput.size()) == (ssize)bufferedOutput.size());
    bufferedOutput.free();
  }
#endif

  void promptWrite(usize offset = 0, const String& clearStr = String())
  {
    offset += offset / stdoutScreenWidth * 2;
    Array<conchar> buffer(prompt.size() + input.size() + clearStr.length());
    buffer.append(prompt);
    buffer.append(input);
    ASSERT(clearStr.isEmpty() || clearStr.length() == 1);
    if(!clearStr.isEmpty())
      buffer.append(*(const tchar*)clearStr);
    Array<conchar> wrappedBuffer(buffer.size() + buffer.size() / stdoutScreenWidth * 2 + 2);
    for(usize i = 0, len = buffer.size(); i < len; i += stdoutScreenWidth)
    {
      usize lineEnd = len - i;
      if(lineEnd > stdoutScreenWidth)
        lineEnd = stdoutScreenWidth;
      wrappedBuffer.append((const conchar*)buffer + i, lineEnd);
      if(lineEnd == stdoutScreenWidth)
      {
        wrappedBuffer.append(_T('\r'));
        wrappedBuffer.append(_T('\n'));
      }
    }
#ifdef _MSC_VER
    writeConsole((const conchar*)wrappedBuffer + offset, wrappedBuffer.size() - offset);
#else
    String dataToWrite((wrappedBuffer.size() - offset) * sizeof(uint32) + 10);
    dataToWrite.append("\x1b[?7l");
    VERIFY(Unicode::append((const conchar*)wrappedBuffer + offset, wrappedBuffer.size() - offset, dataToWrite));
    dataToWrite.append("\x1b[?7h");
    writeConsole(dataToWrite, dataToWrite.length());
#endif
    if(caretPos < input.size() + clearStr.length())
      moveCursorPosition(prompt.size() + input.size() + clearStr.length(), -(ssize)(input.size() + clearStr.length() - caretPos));
#ifdef _MSC_VER
    else // enforce cursor blink timer reset
      moveCursorPosition(prompt.size() + input.size() + clearStr.length(), 0);
#endif
  }

  void promptInsert(conchar character)
  {
    if(caretPos != input.size())
    {
      Array<conchar> newInput(input.size() + 1);
      newInput.append(input,  caretPos);
      newInput.append(character);
      newInput.append((conchar*)input + caretPos, input.size() - caretPos);
      input.swap(newInput);
    }
    else
      input.append(character);
    usize oldCaretPos = caretPos;
    ++caretPos;
    promptWrite(prompt.size() + oldCaretPos);
  }

  void promptRemove()
  {
    if(!input.isEmpty() && caretPos > 0)
    {
      if(caretPos != input.size())
      {
        Array<conchar> newInput(input.size() - 1);
        newInput.append(input,  caretPos -1);
        newInput.append((conchar*)input + caretPos, input.size() - caretPos);
        input.swap(newInput);
      }
      else
        input.resize(input.size() - 1);
      promptMoveLeft();
      promptWrite(prompt.size() + caretPos, _T(" "));
    }
  }

  void promptRemoveNext()
  {
    if(caretPos < input.size())
    {
      if(caretPos != input.size() - 1)
      {
        Array<conchar> newInput(input.size() - 1);
        newInput.append(input,  caretPos);
        newInput.append((conchar*)input + caretPos + 1, input.size() - (caretPos + 1));
        input.swap(newInput);
      }
      else
        input.resize(input.size() - 1);
      promptWrite(prompt.size() + caretPos, _T(" "));
    }
  }

  void promptMoveLeft()
  {
    if(caretPos > 0)
    {
      moveCursorPosition(prompt.size() + caretPos, -1);
      --caretPos;
    }
  }

  void promptMoveHome()
  {
    if(caretPos > 0)
    {
      moveCursorPosition(prompt.size() + caretPos, -(ssize)caretPos);
      caretPos = 0;
    }
  }

  void promptMoveRight()
  {
    if(caretPos < input.size())
    {
      moveCursorPosition(prompt.size() + caretPos, 1);
      ++caretPos;
    }
  }

  void promptMoveEnd()
  {
    if(caretPos < input.size())
    {
      moveCursorPosition(prompt.size() + caretPos, input.size() - caretPos);
      caretPos = input.size();
    }
  }

  void promptHistoryUp()
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

  void promptHistoryDown()
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

  void promptClear()
  {
    usize bufferLen = prompt.size() + input.size();
    usize additionalLines = bufferLen / stdoutScreenWidth;
    if(additionalLines)
    {
      moveCursorPosition(prompt.size() + caretPos, -(ssize)(caretPos + prompt.size()));
      String clearLine(stdoutScreenWidth + 2);
      for(usize i = 0; i < stdoutScreenWidth; ++i)
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
      for(usize i = 0; i < additionalLines; ++i)
        clearCmd.append(clearLine);
      for(usize i = 0, count = bufferLen - additionalLines * stdoutScreenWidth; i < count; ++i)
        clearCmd.append(_T(' '));
#ifndef _MSC_VER
      clearCmd.append("\x1b[?7h");
#endif
      writeConsole(clearCmd, clearCmd.length());
      moveCursorPosition(prompt.size() + input.size(), -(ssize)bufferLen);
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
      for(usize i = 0, count = bufferLen; i < count; ++i)
        clearCmd.append(_T(' '));
      clearCmd.append(_T('\r'));
#ifndef _MSC_VER
      clearCmd.append("\x1b[?7h");
#endif
      writeConsole(clearCmd, clearCmd.length());
    }
  }

  void saveCursorPosition()
  {
    usize x, y;
    getCursorPosition(x, y);
    stdoutCursorX = x;
    if(stdoutCursorX)
      writeConsole(_T("\r\n"), 2);
  }

  void restoreCursorPosition()
  {
    if(stdoutCursorX)
    {
#ifdef _MSC_VER
      CONSOLE_SCREEN_BUFFER_INFO csbi;
      VERIFY(GetConsoleScreenBufferInfo(hOriginalStdOut, &csbi));
      csbi.dwCursorPosition.X = (SHORT)stdoutCursorX;
      --csbi.dwCursorPosition.Y;
      VERIFY(SetConsoleCursorPosition(hOriginalStdOut, csbi.dwCursorPosition));
#else
      String moveCmd;
      moveCmd.printf("\x1b[A\r\x1b[%dC", (int)stdoutCursorX);
      writeConsole(moveCmd, moveCmd.length());
#endif
    }
  }

  void restoreTerminalMode()
  {
#ifdef _MSC_VER
    VERIFY(SetConsoleMode(hOriginalStdOut, consoleOutputMode));
#else
    VERIFY(tcsetattr(originalStdout, TCSADRAIN, &noEchoMode) == 0);
#endif
  }

  void enableTerminalRawMode()
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
    HANDLE handles[] = {hStdIn, outOverlapped.hEvent, errOverlapped.hEvent};
    while(!inputComplete)
    {
      DWORD dw = WaitForMultipleObjects(3, handles, FALSE, INFINITE);
      switch(dw)
      {
      case WAIT_OBJECT_0:
        {
          INPUT_RECORD records[1];
          DWORD read;
          VERIFY(ReadConsoleInput(hStdIn, records, 1, &read));
          usize keyEvents = 0;
          for(DWORD i = 0; i < read; ++i)
            switch(records[i].EventType)
            {
            case WINDOW_BUFFER_SIZE_EVENT:
              handleResize(records[i].Event.WindowBufferSizeEvent.dwSize.X);
              break;
            case KEY_EVENT:
              {
                const KEY_EVENT_RECORD& keyEvent = records[i].Event.KeyEvent;
                if(keyEvent.bKeyDown)
#ifdef _UNICODE
                  handleInput(keyEvent.uChar.UnicodeChar, keyEvent.wVirtualKeyCode);
#else
                  handleInput(keyEvent.uChar.AsciiChar, keyEvent.wVirtualKeyCode);
#endif
              }
              break;
            }
        }
        break;
      case WAIT_OBJECT_0 + 1:
        handleOutput(hStdOutRead, hOriginalStdOut, stdoutBuffer, sizeof(stdoutBuffer), outOverlapped);
        break;
      case WAIT_OBJECT_0 + 2:
        handleOutput(hStdErrRead, hOriginalStdErr, stderrBuffer, sizeof(stderrBuffer), errOverlapped);
        break;
      default:
        break;
      }
    }

#else
    for(;;)
    {
      while(!bufferedInput.isEmpty())
      {
        char buffer[64];
        usize len = readChar(buffer);
        handleInput(buffer, len);
        if(inputComplete)
          break;
      }
      if(inputComplete)
        break;

      fd_set fdr;
      FD_ZERO(&fdr);
      FD_SET(stdoutRead, &fdr);
      FD_SET(stderrRead, &fdr);
      FD_SET(STDIN_FILENO, &fdr);
      FD_SET(resizeEventFd, &fdr);
      int nfds = stdoutRead;
      if(stderrRead > nfds)
        nfds = stderrRead;
      if(resizeEventFd > nfds)
        nfds = resizeEventFd;
      timeval tv = {1000000, 0};
      flushConsole();
      switch(select(nfds + 1, &fdr, 0, 0, &tv))
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
        handleOutput(stdoutRead, originalStdout);
      if(FD_ISSET(stderrRead, &fdr))
        handleOutput(stderrRead, originalStderr);
      if(FD_ISSET(STDIN_FILENO, &fdr))
      {
        char buffer[64];
        usize len = readChar(buffer);
        handleInput(buffer, len);
      }
      if(FD_ISSET(resizeEventFd, &fdr))
      {
        uint64 buffer;
        if(read(resizeEventFd, &buffer, sizeof(uint64)) == sizeof(uint64))
          handleResize(getScreenWidth());
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

  void redirectPendingData()
  {
#ifdef _MSC_VER
    DWORD read;
    while(GetOverlappedResult(hStdOutRead, &outOverlapped, &read, FALSE))
    {
        DWORD written;
        VERIFY(WriteConsole(hOriginalStdOut, stdoutBuffer, read / sizeof(tchar), &written, NULL));
        ASSERT(written == read / sizeof(tchar));
        while(ReadFile(hStdOutRead, stdoutBuffer, sizeof(stdoutBuffer), &read, &outOverlapped))
        {
          VERIFY(WriteConsole(hOriginalStdOut, stdoutBuffer, read / sizeof(tchar), &written, NULL));
          ASSERT(written == read / sizeof(tchar));
        }
    }
    while(GetOverlappedResult(hStdErrRead, &errOverlapped, &read, FALSE))
    {
        DWORD written;
        VERIFY(WriteConsole(hOriginalStdErr, stderrBuffer, read / sizeof(tchar), &written, NULL));
        ASSERT(written == read / sizeof(tchar));
        while(ReadFile(hStdErrRead, stderrBuffer, sizeof(stderrBuffer), &read, &errOverlapped))
        {
          VERIFY(WriteConsole(hOriginalStdErr, stderrBuffer, read / sizeof(tchar), &written, NULL));
          ASSERT(written == read / sizeof(tchar));
        }
    }
#else
    for(;;)
    {
      fd_set fdr;
      FD_ZERO(&fdr);
      FD_SET(stdoutRead, &fdr);
      FD_SET(stderrRead, &fdr);
      timeval tv = {0, 0};
      switch(select((stdoutRead > stderrRead ? stdoutRead : stderrRead) + 1, &fdr, 0, 0, &tv))
      {
      case -1:
        ASSERT(false);
        return;
      case 0:
        break;
      default: // todo: redirect data character by character to ensure utf8 and escaped sequences are valid?
        if(FD_ISSET(stdoutRead, &fdr))
        {
          char buffer[4096];
          ssize i = read(stdoutRead, buffer, sizeof(buffer));
          VERIFY(i != -1);
          VERIFY(write(originalStdout, buffer, i) == i);
        }
        if(FD_ISSET(stderrRead, &fdr))
        {
          char buffer[4096];
          ssize i = read(stderrRead, buffer, sizeof(buffer));
          VERIFY(i != -1);
          VERIFY(write(originalStderr, buffer, i) == i);
        }
        continue;
      }
      break;
    }
#endif
  }

  void handleResize(usize newWidth)
  {
    promptClear();
    stdoutScreenWidth = newWidth;
    promptWrite();
  }

#ifdef _MSC_VER
  void handleInput(tchar ch, WORD virtualKeyCode)
  {
    switch(ch)
    {
    case _T('\0'):
      switch(virtualKeyCode)
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
      promptInsert(ch);
    }
  }
#else
  void handleInput(char* input, usize len)
  {
    if(*input == '\x1b')
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
    else
    {
      uint32 ch = utf8 ? Unicode::fromString(input, len) : *(uchar*)input;
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
  }
#endif

#ifdef _MSC_VER
  void handleOutput(HANDLE hStdRead, HANDLE hOriginalStd, char* buffer, usize bufferSize, OVERLAPPED& overlapped)
#else
  void handleOutput(int fd, int originalFd)
#endif
  {
#ifdef _MSC_VER
    // get new output
    DWORD read;
    if(!GetOverlappedResult(hStdRead, &overlapped, &read, FALSE))
      return;
#endif

    // restore console
    promptClear();
    restoreCursorPosition();
    restoreTerminalMode();

    // add new output
#ifdef _MSC_VER
    DWORD written;
    VERIFY(WriteConsole(hOriginalStd, buffer, read / sizeof(tchar), &written, NULL));
    ASSERT(written == read / sizeof(tchar));
    while(ReadFile(hStdRead, buffer, (DWORD)bufferSize, &read, &overlapped))
    {
      VERIFY(WriteConsole(hOriginalStd, buffer, read / sizeof(tchar), &written, NULL));
      ASSERT(written == read / sizeof(tchar));
    }
#else
    char buffer[4096];
    ssize i = read(fd, buffer, sizeof(buffer));
    VERIFY(i != -1);
    writeConsole(buffer, i); // todo: write to originalFd and ensure utf8 sequences are valid
    flushConsole();
#endif

    // add prompt
    enableTerminalRawMode();
    saveCursorPosition();
    promptWrite();
  }

#ifndef _MSC_VER
  usize readChar(char* buffer)
  {
    if(utf8)
      return readBufferedUtf8Char(buffer);
    return readBufferedChar(buffer);
  }

  usize readNextUnbufferedEscapedSequence(char* buffer, char firstChar, char lastChar)
  {
    for(char ch;;)
    {
      VERIFY(read(STDIN_FILENO, &ch, 1) == 1);
      if(ch == '\x1b')
      {
        *buffer = ch;
        buffer[1] = 0;
        usize len = 1 + readUnbufferedEscapedSequence(buffer, buffer + 1);
        char sequenceType = buffer[len - 1];
        if(sequenceType != lastChar || buffer[1] != firstChar)
        {
          bufferedInput.append((byte*)buffer, len);
          continue;
        }
        return len;
      }
      else
        bufferedInput.append((byte*)&ch, 1);
    }
  }

  usize readBufferedUtf8Char(char* buffer)
  {
    if(bufferedInput.isEmpty())
    {
      char ch;
      VERIFY(read(STDIN_FILENO, &ch, 1) == 1);
      if(ch == '\x1b')
      {
        *buffer = '\x1b';
        return 1 + readUnbufferedEscapedSequence(buffer, buffer + 1);
      }
      *buffer = ch;
      return readUnbufferedUtf8Char(buffer, buffer + 1, Unicode::length(ch));
    }
    *buffer = *(const char*)(const byte*)bufferedInput;
    bufferedInput.removeFront(1);
    if(*buffer == '\x1b')
      return 1 + readBufferedEscapedSequence(buffer, buffer + 1);
    char* start = buffer;
    usize len = Unicode::length(*(buffer++));
    while((usize)(buffer - start) < len)
    {
      if(bufferedInput.isEmpty())
        return readUnbufferedUtf8Char(start, buffer, len);
      *buffer = *(const char*)(const byte*)bufferedInput;
      bufferedInput.removeFront(1);
      if(*buffer == '\x1b')
      {
        Buffer incompleteUtf8((const byte*)start, buffer - start);
        *start = '\x1b';
        usize result = 1 + readBufferedEscapedSequence(start, start + 1);
        bufferedInput.prepend(incompleteUtf8);
        return result;
      }
      ++buffer;
    }
    *buffer = '\0';
    return len;
  }

  usize readUnbufferedUtf8Char(char* start,  char* buffer, usize len)
  {
    while((usize)(buffer - start) < len)
    {
      VERIFY(read(STDIN_FILENO, buffer, 1) == 1);
      ++buffer;
    }
    *buffer = '\0';
    return len;
  }

  usize readBufferedChar(char* buffer)
  {
    if(bufferedInput.isEmpty())
      return readUnbufferedChar(buffer);
    *buffer = *(const char*)(const byte*)bufferedInput;
    bufferedInput.removeFront(1);
    if(*buffer == '\x1b')
      return 1 + readBufferedEscapedSequence(buffer, buffer + 1);
    buffer[1] = '\0';
    return 1;
  }

  usize readUnbufferedChar(char* buffer)
  {
    VERIFY(read(STDIN_FILENO, buffer, 1) == 1);
    if(*buffer == '\x1b')
      return 1 + readUnbufferedEscapedSequence(buffer, buffer + 1);
    buffer[1] = '\0';
    return 1;
  }

  static bool isSeqAttributeChar(char ch) {return isdigit(ch) || ch == ';' || ch == '?' || ch == '[';}

  usize readBufferedEscapedSequence(const char* seqStart, char* buffer)
  {
    for(char* start = buffer;;)
    {
      if(bufferedInput.isEmpty())
        return buffer - start + readUnbufferedEscapedSequence(seqStart, buffer);
      *buffer = *(const char*)(const byte*)bufferedInput;
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

  usize readUnbufferedEscapedSequence(const char* seqStart, char* buffer)
  {
    for(char* start = buffer, ch;;)
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
#endif

  void concharArrayToString(const conchar* data, usize len, String& result)
  {
    result.clear();
#ifdef _MSC_VER
    result.append(data, len);
#else
    if(utf8)
    {
      result.reserve(len * sizeof(uint32));
      Unicode::append(data, len, result);
    }
    else
    {
      result.reserve(len);
      for(const conchar* i = data, * end = data + len; i < end; ++i)
        result.append((const tchar&)*i);
    }
#endif
  }

  void stringToConcharArray(const String& data, Array<conchar>& result)
  {
    result.clear();
    result.reserve(data.length());
#ifdef _MSC_VER
    for(const tchar* i = data, * end = i + data.length(); i < end; ++i)
      result.append(*i);
#else
    if(utf8)
    {
      for(const tchar* i = data, * end = i + data.length(); i < end;)
      {
        usize len = Unicode::length(*i);
        if(len == 0 || (usize)(end - i) < len)
        {
          ++i;
          continue;
        }
        result.append(Unicode::fromString(i, len));
        i += len;
      }
    }
    else
      for(const tchar* i = data, * end = i + data.length(); i < end; ++i)
        result.append((const uchar&)*i);
#endif
  }

private:
  bool valid;
#ifdef _MSC_VER
  HANDLE hStdOutRead;
  HANDLE hStdOutWrite;
  HANDLE hStdErrRead;
  HANDLE hStdErrWrite;
  HANDLE hOriginalStdOut;
  HANDLE hOriginalStdErr;
  int originalStdout;
  int originalStderr;
  int newStdout;
  int newStderr;
  OVERLAPPED outOverlapped;
  OVERLAPPED errOverlapped;
  char stdoutBuffer[4096];
  char stderrBuffer[4096];
  DWORD consoleOutputMode;
  DWORD consoleInputMode;
#else
  bool utf8;
  static int originalStdout;
  int originalStderr;
  int stdoutRead;
  int stdoutWrite;
  int stderrRead;
  int stderrWrite;

  static bool originalTermiosValid;
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

  usize stdoutScreenWidth;
  usize stdoutCursorX;

  Array<conchar> prompt;
  Array<conchar> input;
  bool inputComplete;
  usize caretPos;

  List<String> history;
  List<String>::Iterator historyPos;
  bool historyRemoveLast;
};

#ifndef _MSC_VER
int Console::Prompt::Private::originalStdout = -1;
bool Console::Prompt::Private::originalTermiosValid = false;
termios Console::Prompt::Private::originalTermios;
int Console::Prompt::Private::resizeEventFd = 0;
#ifdef __CYGWIN__
int Console::Prompt::Private::resizeEventFdWrite = 0;
#endif
#endif

Console::Prompt::Prompt() : p(new Private) {}
Console::Prompt::~Prompt() {delete p;}
String Console::Prompt::getLine(const String& str) {return p->getLine(str);}
