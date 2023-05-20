
#include <nstd/Console.hpp>

void testConsole()
{
  // test printf
  {
    Console::printf(_T("%s\n"), _T("Hello world"));
    char buffer[5000 * 4];
    usize bufferSize = sizeof(buffer) - 1;
    Memory::fill(buffer, 'a', bufferSize - 1);
    buffer[bufferSize - 2] = 'b';
    buffer[bufferSize - 1] = '\0';
    Console::printf(_T("%hs%hs\n"), buffer, buffer);
  }
}

int main(int argc, char* argv[])
{
    testConsole();
    return 0;
}
