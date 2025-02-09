
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
      Console::errorf("%d%s", counter, ((counter + 1) % 3) == 0 ? "\n" : " ");
    else
      Console::printf("%d%s", counter, ((counter + 1) % 3) == 0 ? "\n" : " ");
    ++counter;
  }
  return 0;
}

int main(int argc, char* argv[])
{
  String password("root");
  String user("root");
  String address("127.0.0.1:13211");
  {
    Process::Option options[] = {
        {'p', "password", Process::argumentFlag},
        {'u', "user", Process::argumentFlag},
        {'h', "help", Process::optionFlag},
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
        Console::errorf("Unknown option: %s.\n", (const char*)argument);
        return 1;
      case ':':
        Console::errorf("Option %s required an argument.\n", (const char*)argument);
        return 1;
      default:
        Console::errorf("Usage: %s [-u <user>] [-p <password>] [<address>]\n", argv[0]);
        return 1;
      }
  }

  Console::printf("user=%s, password=%s, address=%s\n",
    (const char*)user, (const char*)password, (const char*)address);

  Console::printf("Hello World!\n");

  Console::Prompt prompt;

  Log::infof("Hello World!");
  Log::warningf("%s", "This is a warning!");
  
  Thread thread;
  Signal termSignal;
  thread.start(threadProc, &termSignal);
  for(;;)
  {
    String result = prompt.getLine("test> ");
    Console::printf("input: %s\n", (const char*)result);
    if(result == "exit")
      break;
  }
  termSignal.set();
  thread.join();

  return 0;
}
