
#include <nstd/Debug.h>
#include <nstd/Buffer.h>

void_t testBuffer()
{
  Buffer buffer1;
  Buffer buffer2;
  Buffer buffer3;
  ASSERT(buffer1 == buffer2);
  buffer1 = buffer2;
  ASSERT(buffer1 == buffer2);
  buffer3.assign((byte_t*)"123", 3);
  buffer3 = buffer2;
  ASSERT(buffer3 == buffer2);
}
