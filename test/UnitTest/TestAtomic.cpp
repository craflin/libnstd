
#include <nstd/Debug.h>
#include <nstd/Atomic.h>

void testAtomic()
{
  volatile usize size = 0;
  ASSERT(Atomic::increment(size) == 1);
  ASSERT(Atomic::increment(size) == 2);
  ASSERT(Atomic::increment(size) == 3);
  ASSERT(Atomic::decrement(size) == 2);
  ASSERT(Atomic::decrement(size) == 1);
  ASSERT(Atomic::decrement(size) == 0);
  volatile int32 int32 = 0;
  ASSERT(Atomic::increment(int32) == 1);
  ASSERT(Atomic::decrement(int32) == 0);
  ASSERT(Atomic::decrement(int32) == -1);
  volatile uint32 uint32 = 0xfffffff0;
  ASSERT(Atomic::increment(uint32) == 0xfffffff1);
  ASSERT(Atomic::decrement(uint32) == 0xfffffff0);
  volatile int64 int64 = 0;
#ifndef _ARM  
  ASSERT(Atomic::increment(int64) == 1);
  ASSERT(Atomic::decrement(int64) == 0);
  ASSERT(Atomic::decrement(int64) == -1);
#endif
  volatile uint64 uint64 =  (0xffffffffULL << 32) | 0xfffffff0ULL;
#ifndef _ARM  
  ASSERT(Atomic::increment(uint64) == ((0xffffffffULL << 32) | 0xfffffff1ULL));
  ASSERT(Atomic::decrement(uint64) == ((0xffffffffULL << 32) | 0xfffffff0ULL));
#endif

  void* volatile ptr = 0;
  ASSERT(Atomic::compareAndSwap(ptr, 0, (void*)0xff) == 0);
  ASSERT(Atomic::compareAndSwap(ptr, (void*)0xff, 0) == (void*)0xff);
  ASSERT(Atomic::compareAndSwap(ptr, (void*)23, 0) == 0);
  
  size = 0;
  ASSERT(Atomic::swap(size, 1) == 0);
  ASSERT(Atomic::swap(size, 2) == 1);
  int32 = 0;
  ASSERT(Atomic::swap(int32, 1) == 0);
  ASSERT(Atomic::swap(int32, -1) == 1);
  ASSERT(Atomic::swap(int32, 2) == -1);
  int64 = 0;
  ASSERT(Atomic::swap(int64, 1) == 0);
  ASSERT(Atomic::swap(int64, -1) == 1);
  ASSERT(Atomic::swap(int64, 2) == -1);
  uint32 = 0;
  ASSERT(Atomic::swap(uint32, 1) == 0);
  ASSERT(Atomic::swap(uint32, 2) == 1);
  uint64 = 0;
  ASSERT(Atomic::swap(uint64, 1) == 0);
  ASSERT(Atomic::swap(uint64, 2) == 1);
  ptr = 0;
  ASSERT(Atomic::swap(ptr, (void*)0xffffffff) == 0);
  ASSERT(Atomic::swap(ptr, (void*)123) == (void*)0xffffffff);
}
