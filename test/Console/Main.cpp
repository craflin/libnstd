
#include <nstd/Console.hpp>
#include <nstd/Process.hpp>

#include <nstd/Thread.hpp>
#include <nstd/Signal.hpp>
#include <nstd/Log.hpp>

uint threadProc(void* param)
{
  Signal* termSignal = (Signal*)param;
  int counter = 0;
  while(!termSignal->wait(1000))
  {
    if(counter % 5 == 0)
      Console::errorf(_T("%d%s"), counter, ((counter + 1) % 3) == 0 ? _T("\n") : _T(" "));
    else
      Console::printf(_T("%d%s"), counter, ((counter + 1) % 3) == 0 ? _T("\n") : _T(" "));
    ++counter;
  }
  return 0;
}

int main(int argc, tchar* argv[])
{
  String password(_T("root"));
  String user(_T("root"));
  String address(_T("127.0.0.1:13211"));
  {
    Process::Option options[] = {
        {_T('p'), _T("password"), Process::argumentFlag},
        {_T('u'), _T("user"), Process::argumentFlag},
        {_T('h'), _T("help"), Process::optionFlag},
    };
    Process::Arguments arguments(argc, argv, options);
    int character;
    String argument;
    while(arguments.read(character, argument))
      switch(character)
      {
      case 'p':
        password = argument;
        break;
      case 'u':
        user = argument;
        break;
      case 0:
        address = argument;
        break;
      case '?':
        Console::errorf(_T("Unknown option: %s.\n"), (const tchar*)argument);
        return 1;
      case ':':
        Console::errorf(_T("Option %s required an argument.\n"), (const tchar*)argument);
        return 1;
      default:
        Console::errorf(_T("Usage: %s [-u <user>] [-p <password>] [<address>]\n"), argv[0]);
        return 1;
      }
  }

  Console::printf(_T("user=%s, password=%s, address=%s\n"),
    (const tchar*)user, (const tchar*)password, (const tchar*)address);

  Console::printf(_T("Hello World!\n"));

  Console::Prompt prompt;

  Log::infof(_T("Hello World!"));
  Log::warningf(_T("%s"), _T("This is a warning!"));
  
  Thread thread;
  Signal termSignal;
  thread.start(threadProc, &termSignal);
  for(;;)
  {
    String result = prompt.getLine(_T("test> "));
    Console::printf(_T("input: %s\n"), (const tchar*)result);
    if(result == _T("exit"))
      break;
  }
  termSignal.set();
  thread.join();

  return 0;
}
