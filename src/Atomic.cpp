
#include <nstd/Atomic.h>

#if defined(_MSC_VER) && !defined(_WIN64)

#include <Windows.h>

int64_t Atomic::increment(volatile int64_t& var)
{
  return InterlockedIncrement64((__int64 volatile *)&var);
}

uint64_t Atomic::increment(volatile uint64_t& var)
{
  return InterlockedIncrement64((__int64 volatile *)&var);
}

int64_t Atomic::decrement(volatile int64_t& var)
{
  return InterlockedDecrement64((__int64 volatile *)&var);
}

uint64_t Atomic::decrement(volatile uint64_t& var)
{
  return InterlockedDecrement64((__int64 volatile *)&var);
}

#endif
