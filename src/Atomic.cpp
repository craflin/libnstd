
#include <nstd/Atomic.h>

#if defined(_MSC_VER) && !defined(_WIN64)

#include <Windows.h>

int64_t Atomic::increment(volatile int64_t& var)
{
  return InterlockedIncrement64((__int64 volatile *)&var);
}

uint64_t Atomic::increment(volatile uint64_t& var)
{
  return (uint64_t)InterlockedIncrement64((__int64 volatile *)&var);
}

int64_t Atomic::decrement(volatile int64_t& var)
{
  return InterlockedDecrement64((__int64 volatile *)&var);
}

uint64_t Atomic::decrement(volatile uint64_t& var)
{
  return (uint64_t)InterlockedDecrement64((__int64 volatile *)&var);
}

int64_t Atomic::compareAndSwap(int64_t volatile& var, int64_t oldVal, int64_t newVal)
{
  return (uint64_t)InterlockedCompareExchange((unsigned __int64 volatile*)&var, (unsigned __int64)newVal, (unsigned __int64)oldVal);
}

uint64_t Atomic::compareAndSwap(uint64_t volatile& var, uint64_t oldVal, uint64_t newVal)
{
  return InterlockedCompareExchange(&var, newVal, oldVal);
}

#endif
