
#include <nstd/Debug.h>
#include <nstd/Memory.h>

#include <cstring>

void testMemory()
{
  // test alloc small
  {
    usize size;
    void* buffer = Memory::alloc(123, size);
    ASSERT(buffer);
    ASSERT(size >= 123);
    ASSERT(Memory::size(buffer) == size);
    Memory::free(buffer);
    buffer = Memory::alloc(123, size);
    ASSERT(size >= 123);
  
    char testBuffer[100];
    memset(testBuffer, 'a', sizeof(testBuffer));
    Memory::fill(buffer, 'b', size);
    ASSERT(Memory::compare(buffer, testBuffer, sizeof(testBuffer)) != 0);
    ASSERT(Memory::compare(buffer, testBuffer, sizeof(testBuffer)) == memcmp(buffer, testBuffer, sizeof(testBuffer)));
    ASSERT(Memory::compare(testBuffer, buffer, sizeof(testBuffer)) == memcmp(testBuffer, buffer, sizeof(testBuffer)));
    Memory::fill(buffer, 'a', size);
    ASSERT(Memory::compare(buffer, testBuffer, sizeof(testBuffer)) == 0);

    Memory::free(buffer);
  }

  // test alloc large
  {
    usize size;
    void* buffer = Memory::alloc(50000 * 5, size);
    ASSERT(buffer);
    ASSERT(size >= 50000 * 5);
    Memory::free(buffer);
    buffer = Memory::alloc(50000 * 5, size);
    ASSERT(size >= 50000 * 5);
    Memory::free(buffer);
  }
}
