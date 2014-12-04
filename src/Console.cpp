
#include <cstdarg>
#include <cstdio>
#ifdef _MSC_VER
#include <tchar.h>
#endif
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <nstd/Debug.h>
#include <conio.h>
#else
// todo
#endif

#include <nstd/Console.h>

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

#ifdef _WIN32
class ConsolePromptPrivate
{
public:
  ConsolePromptPrivate() : valid(false)
  {
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
  }

  ~ConsolePromptPrivate()
  {
    if(!valid)
      return;
    CancelIo(hStdOutRead);
    CloseHandle(overlapped.hEvent);
    _dup2(originalStdout, _fileno(stdout));
    _close(originalStdout); // this should close hOriginalStdOut
    _close(newStdout); // this should close hStdOutWrite
    CloseHandle(hStdOutRead);
  }

  size_t getScreenWidth()
  {
    return csbi.dwSize.X;
  }

  void_t getCursorPosition(size_t& x, size_t& y)
  {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    VERIFY(GetConsoleScreenBufferInfo(hOriginalStdOut, &csbi));
    x = csbi.dwCursorPosition.X;
    y = csbi.dwCursorPosition.Y;
  }

  void_t setCursorPosition(size_t x, size_t y)
  {
    COORD pos = {(SHORT)x, (SHORT)y};
    VERIFY(SetConsoleCursorPosition(hOriginalStdOut, pos));
  }

  void_t moveCursorPosition(size_t from, ssize_t x)
  {
    size_t width = getScreenWidth();
    size_t to = from + x;
    size_t oldY = from / width;
    size_t oldX = from % width;
    size_t newY = to / width;
    size_t newX = to % width;
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
  }

  void_t writeConsole(const tchar_t* data, size_t len)
  {
    DWORD written;
    VERIFY(WriteConsole(hOriginalStdOut, data, (DWORD)len, &written, NULL));
    ASSERT(written == (DWORD)len);
  }

  void_t promptWrite(size_t offset = 0, const String& clearStr = String())
  {
    size_t width = getScreenWidth();
    offset += offset / width * 2;
    String buffer(prompt.length() + input.length() + clearStr.length());
    buffer.append(prompt);
    buffer.append(input);
    buffer.append(clearStr);
    String wrappedBuffer(buffer.length() + buffer.length() / width * 2 + 2);
    for(size_t i = 0, len = buffer.length(); i < len; i += width)
    {
      size_t lineEnd = len - i;
      if(lineEnd > width)
        lineEnd = width;
      wrappedBuffer.append((const tchar_t*)buffer + i, lineEnd);
      if(lineEnd == width)
      {
        wrappedBuffer.append(_T('\r'));
        wrappedBuffer.append(_T('\n'));
      }
    }
    writeConsole((const tchar_t*)wrappedBuffer + offset, wrappedBuffer.length() - offset);
    if(caretPos < input.length() + clearStr.length())
      moveCursorPosition(prompt.length() + input.length() + clearStr.length(), -(ssize_t)(input.length() + clearStr.length() - caretPos));
    else // enforce cursor blink reset
      moveCursorPosition(prompt.length() + input.length() + clearStr.length(), 0);
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
    size_t width = getScreenWidth();
    size_t bufferLen = prompt.length() + input.length();
    size_t additionalLines = bufferLen / width;
    if(additionalLines)
    {
      moveCursorPosition(prompt.length() + caretPos, -(ssize_t)(caretPos + prompt.length()));
      String clearLine(width + 2);
      for(size_t i = 0; i < width; ++i)
        clearLine.append(_T(' '));
      clearLine.append(_T("\n\r"));
      String clearCmd(bufferLen + additionalLines * 2);
      for(size_t i = 0; i < additionalLines; ++i)
        clearCmd.append(clearLine);
      for(size_t i = 0, count = bufferLen - additionalLines * width; i < count; ++i)
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

  String getLine(const String& prompt)
  {
    if(!valid)
    {
      // todo: print std and read line from stdin 
      return String();
    }

    // disable eol wrap
    VERIFY(SetConsoleMode(hOriginalStdOut, ENABLE_PROCESSED_OUTPUT));

    // save console cursor position
    VERIFY(GetConsoleScreenBufferInfo(hOriginalStdOut, &csbi));

    // move cursor to the next line
    DWORD written;
    bool addedNewLine = false;
    if(csbi.dwCursorPosition.X != 0)
    {
      VERIFY(WriteConsole(hOriginalStdOut, (const tchar_t*)_T("\r\n"), 2, &written, NULL));
      ASSERT(written == 2);
      addedNewLine = true;
    }

    // write prompt
    this->prompt = prompt;
    input.clear();
    caretPos = 0;
    promptWrite();

    // wait for io
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE handles[] = {hStdIn, overlapped.hEvent};
    for(;;)
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
                goto returnResult;
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

          // clear prompt line
          promptClear();

          // restore cursor position
          if(addedNewLine)
          {
            SHORT x = csbi.dwCursorPosition.X;
            VERIFY(GetConsoleScreenBufferInfo(hOriginalStdOut, &csbi));
            csbi.dwCursorPosition.X = x;
            --csbi.dwCursorPosition.Y;
            VERIFY(SetConsoleCursorPosition(hOriginalStdOut, csbi.dwCursorPosition));
          }

          // restore console mode
          VERIFY(SetConsoleMode(hOriginalStdOut, consoleMode));

          // add new output
          VERIFY(WriteFile(hOriginalStdOut, stdoutBuffer, read, &written, NULL));
          ASSERT(written == read);
          while(ReadFile(hStdOutRead, stdoutBuffer, sizeof(stdoutBuffer), &read, &overlapped))
          {
            VERIFY(WriteFile(hOriginalStdOut, stdoutBuffer, read, &written, NULL));
            ASSERT(written == read);
          }

          // set console mode back to no eol wrap
          VERIFY(SetConsoleMode(hOriginalStdOut, ENABLE_PROCESSED_OUTPUT));

          // get new cursor position
          VERIFY(GetConsoleScreenBufferInfo(hOriginalStdOut, &csbi));

          // move cursor to the next line
          addedNewLine = false;
          if(csbi.dwCursorPosition.X != 0)
          {
            VERIFY(WriteConsole(hOriginalStdOut, (const tchar_t*)_T("\r\n"), 2, &written, NULL));
            ASSERT(written == 2);
            addedNewLine = true;
          }

          // write prompt
          promptWrite();
        }
        break;
      case WAIT_OBJECT_0 + 2:
        break;
      }
    }

  returnResult:
    // clear prompt line
    promptClear();

    // restore cursor position
    if(addedNewLine)
    {
      SHORT x = csbi.dwCursorPosition.X;
      VERIFY(GetConsoleScreenBufferInfo(hOriginalStdOut, &csbi));
      csbi.dwCursorPosition.X = x;
      --csbi.dwCursorPosition.Y;
      VERIFY(SetConsoleCursorPosition(hOriginalStdOut, csbi.dwCursorPosition));
    }

    // restore console mode
    VERIFY(SetConsoleMode(hOriginalStdOut, consoleMode));

    return input;
  }

private:
  bool_t valid;
  HANDLE hStdOutRead;
  HANDLE hStdOutWrite;
  HANDLE hOriginalStdOut;
  int originalStdout;
  int newStdout;
  OVERLAPPED overlapped;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  char stdoutBuffer[4096];
  DWORD consoleMode;

  String prompt;
  String input;
  size_t caretPos;
};
#else
#endif

Console::Prompt::Prompt() : data(new ConsolePromptPrivate) {}

Console::Prompt::~Prompt() {delete data;}

String Console::Prompt::getLine(const String& str) {return data->getLine(str);}
