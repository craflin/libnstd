
#include <nstd/Log.hpp>
#include <nstd/Debug.hpp>

void testLogf()
{
  char buf[512 + 1];
  Memory::fill(buf, 'a', 512);
  buf[512] = '\0';
  Log::logf(Log::info, "%s%s", "bbb", buf);
}

int main(int argc, char* argv[])
{
  testLogf();
  return 0;
}
