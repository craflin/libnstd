
#include <nstd/Console.h>
#include <nstd/Process.h>

#include <nstd/Thread.h>
#include <nstd/Signal.h>

uint_t threadProc(void_t* param)
{
  Signal* termSignal = (Signal*)param;
  int_t counter = 0;
  while(!termSignal->wait(3000))
    Console::printf("%d\n", counter++);
  return 0;
}

int_t main(int_t argc, char_t* argv[])
{
  String password("root");
  String user("root");
  String address("127.0.0.1:13211");
  {
    Process::Option options[] = {
        {'p', "password", Process::argumentFlag},
        {'u', "user", Process::argumentFlag},
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
        Console::errorf("Unknown option: %s.\n", (const char_t*)argument);
        return 1;
      case ':':
        Console::errorf("Option %s required an argument.\n", (const char_t*)argument);
        return 1;
      default:
        Console::errorf("Usage: %s [-u <user>] [-p <password>] [<address>]\n");
        return 1;
      }
  }

  Console::printf("user=%s, password=%s, address=%s\n",
    (const char_t*)user, (const char_t*)password, (const char_t*)address);

  Console::printf("Hello World!\n");

  
  Console::Prompt prompt;
  Thread thread;
  Signal termSignal;
  thread.start(threadProc, &termSignal);
  for(;;)
  {
    String result = prompt.getLine("test> ");
    Console::printf("input: %s\n", (const char_t*)result);
    if(result == "exit")
      break;
  }
  termSignal.set();
  thread.join();

  return 0;
}
