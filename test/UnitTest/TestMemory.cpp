
#include <nstd/Debug.h>
#include <nstd/Memory.h>

#include <cstring>

void_t testMemory()
{
  // test alloc small
  {
    size_t size;
    void_t* buffer = Memory::alloc(123, size);
    ASSERT(buffer);
    ASSERT(size >= 123);
    ASSERT(Memory::size(buffer) == size);
    Memory::free(buffer);
    buffer = Memory::alloc(123, size);
    ASSERT(size >= 123);
  
    char_t testBuffer[100];
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
    size_t size;
    void_t* buffer = Memory::alloc(50000 * 5, size);
    ASSERT(buffer);
    ASSERT(size >= 50000 * 5);
    Memory::free(buffer);
    buffer = Memory::alloc(50000 * 5, size);
    ASSERT(size >= 50000 * 5);
    Memory::free(buffer);
  }
}
