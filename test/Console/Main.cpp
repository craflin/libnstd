
#include <nstd/Console.h>
#include <nstd/Process.h>

#include <nstd/Thread.h>
#include <nstd/Signal.h>

uint_t threadProc(void_t* param)
{
  Signal* termSignal = (Signal*)param;
  int_t counter = 0;
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

int_t main(int_t argc, tchar_t* argv[])
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
    int_t character;
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
        Console::errorf(_T("Unknown option: %s.\n"), (const tchar_t*)argument);
        return 1;
      case ':':
        Console::errorf(_T("Option %s required an argument.\n"), (const tchar_t*)argument);
        return 1;
      default:
        Console::errorf(_T("Usage: %s [-u <user>] [-p <password>] [<address>]\n"), argv[0]);
        return 1;
      }
  }

  Console::printf(_T("user=%s, password=%s, address=%s\n"),
    (const tchar_t*)user, (const tchar_t*)password, (const tchar_t*)address);

  Console::printf(_T("Hello World!\n"));

  Console::Prompt prompt;
  Thread thread;
  Signal termSignal;
  thread.start(threadProc, &termSignal);
  for(;;)
  {
    String result = prompt.getLine(_T("test> "));
    Console::printf(_T("input: %s\n"), (const tchar_t*)result);
    if(result == _T("exit"))
      break;
  }
  termSignal.set();
  thread.join();

  return 0;
}
