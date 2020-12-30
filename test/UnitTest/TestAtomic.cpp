
#include <nstd/Atomic.hpp>
#include <nstd/Debug.hpp>

void testAtomic()
{
  {
    volatile int32 int32 = 42;
    volatile uint32 uint32 = 42;
    volatile int64 int64 = 42;
    volatile uint64 uint64 = 42;

    ASSERT(Atomic::increment(int32) == 43);
    ASSERT(Atomic::increment(uint32) == 43);
    ASSERT(Atomic::increment(int64) == 43);
    ASSERT(Atomic::increment(uint64) == 43);

    ASSERT(int32 == 43);
    ASSERT(uint32 == 43);
    ASSERT(int64 == 43);
    ASSERT(uint64 == 43);

    int32 = -100;
    uint32 = 0xfffffff0;
    int64 = -100;
    uint64 = 0xfffffffffffffff0;

    ASSERT(Atomic::increment(int32) == -99);
    ASSERT(Atomic::increment(uint32) == 0xfffffff1);
    ASSERT(Atomic::increment(int64) == -99);
    ASSERT(Atomic::increment(uint64) == 0xfffffffffffffff1);
  }

  {
    volatile int32 int32 = 42;
    volatile uint32 uint32 = 42;
    volatile int64 int64 = 42;
    volatile uint64 uint64 = 42;

    ASSERT(Atomic::decrement(int32) == 41);
    ASSERT(Atomic::decrement(uint32) == 41);
    ASSERT(Atomic::decrement(int64) == 41);
    ASSERT(Atomic::decrement(uint64) == 41);

    ASSERT(int32 == 41);
    ASSERT(uint32 == 41);
    ASSERT(int64 == 41);
    ASSERT(uint64 == 41);
  }

  {
    volatile int32 int32 = 42;
    volatile uint32 uint32 = 42;
    volatile int64 int64 = 42;
    volatile uint64 uint64 = 42;
    void* volatile  ptr = (char*)0 + 42;

    ASSERT(Atomic::compareAndSwap(int32, 42, 0) == 42);
    ASSERT(Atomic::compareAndSwap(uint32, 42, 0) == 42);
    ASSERT(Atomic::compareAndSwap(int64, 42, 0) == 42);
    ASSERT(Atomic::compareAndSwap(uint64, 42, 0) == 42);
    ASSERT(Atomic::compareAndSwap(ptr, (void*)((char*)0 + 42), (void*)0) == (char*)0 + 42);

    ASSERT(int32 == 0);
    ASSERT(uint32 == 0);
    ASSERT(int64 == 0);
    ASSERT(uint64 == 0);
    ASSERT(ptr == 0);

    ASSERT(Atomic::compareAndSwap(int32, 42, 1) == 0);
    ASSERT(Atomic::compareAndSwap(uint32, 42, 1) == 0);
    ASSERT(Atomic::compareAndSwap(int64, 42, 1) == 0);
    ASSERT(Atomic::compareAndSwap(uint64, 42, 1) == 0);
    ASSERT(Atomic::compareAndSwap(ptr, (void*)((char*)0 + 42), (void*)((char*)0 + 1)) == 0);

    ASSERT(int32 == 0);
    ASSERT(uint32 == 0);
    ASSERT(int64 == 0);
    ASSERT(uint64 == 0);
    ASSERT(ptr == 0);
  }

  {
    volatile int32 int32 = 42;
    volatile uint32 uint32 = 42;
    volatile int64 int64 = 42;
    volatile uint64 uint64 = 42;

    ASSERT(Atomic::fetchAndAdd(int32, 3) == 42);
    ASSERT(Atomic::fetchAndAdd(uint32, 3) == 42);
    ASSERT(Atomic::fetchAndAdd(int64, 3) == 42);
    ASSERT(Atomic::fetchAndAdd(uint64, 3) == 42);

    ASSERT(int32 == 45);
    ASSERT(uint32 == 45);
    ASSERT(int64 == 45);
    ASSERT(uint64 == 45);
  }

  {
    volatile int32 int32 = 0;
    ASSERT(Atomic::testAndSet(int32) == 0);
    ASSERT(int32 != 0);
  }

  {
    volatile int32 int32 = 0;
    volatile uint32 uint32 = 0;
    volatile int64 int64 = 0;
    volatile uint64 uint64 = 0;
    void* volatile ptr = 0;
  
    ASSERT(Atomic::swap(int32, 1) == 0);
    ASSERT(Atomic::swap(uint32, 1) == 0);
    ASSERT(Atomic::swap(int64, 1) == 0);
    ASSERT(Atomic::swap(uint64, 1) == 0);
    ASSERT(Atomic::swap(ptr, (void*)((char*)0 + 1)) == 0);

    ASSERT(int32 == 1);
    ASSERT(uint32 == 1);
    ASSERT(int64 == 1);
    ASSERT(uint64 == 1);
    ASSERT(ptr == (char*)0 + 1);
  }

  {
    volatile int32 int32 = 0;
    volatile uint32 uint32 = 0;
    volatile int64 int64 = 0;
    volatile uint64 uint64 = 0;
    void* volatile ptr = 0;
  
    ASSERT(Atomic::load(int32) == 0);
    ASSERT(Atomic::load(uint32) == 0);
    ASSERT(Atomic::load(int64) == 0);
    ASSERT(Atomic::load(uint64) == 0);
    ASSERT(Atomic::load(ptr) == 0);
  }

  {
    volatile int32 int32 = 0;
    volatile uint32 uint32 = 0;
    volatile int64 int64 = 0;
    volatile uint64 uint64 = 0;
    void* volatile ptr = 0;
  
    Atomic::store(int32, 1);
    Atomic::store(uint32, 1);
    Atomic::store(int64, 1);
    Atomic::store(uint64, 1);
    Atomic::store(ptr, (void*)((char*)0 + 1));

    ASSERT(int32 == 1);
    ASSERT(uint32 == 1);
    ASSERT(int64 == 1);
    ASSERT(uint64 == 1);
    ASSERT(ptr == (char*)0 + 1);
  }
}

int main(int argc, char* argv[])
{
    testAtomic();
    return 0;
}
