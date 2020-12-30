
#include <nstd/Memory.hpp>
#include <nstd/Debug.hpp>

void testDebug()
{
  Debug::printf(_T("%s\n"), _T("Hello world"));
  usize bufferSize;
  char* buffer = (char*)Memory::alloc(5000 * 4, bufferSize);
  Memory::fill(buffer, 'a', bufferSize - 1);
  buffer[bufferSize - 2] = 'b';
  buffer[bufferSize - 1] = '\0';
  Debug::printf(_T("%hs%hs\n"), buffer, buffer);
  Memory::free(buffer);
}

int main(int argc, char* argv[])
{
    testDebug();
    return 0;
}
