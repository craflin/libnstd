
#include <nstd/Debug.hpp>
#include <nstd/Buffer.hpp>

void testBuffer()
{
  Buffer buffer1;
  Buffer buffer2;
  Buffer buffer3;
  ASSERT(buffer1 == buffer2);
  buffer1 = buffer2;
  ASSERT(buffer1 == buffer2);
  buffer3.assign((byte*)"123", 3);
  buffer3 = buffer2;
  ASSERT(buffer3 == buffer2);
}
