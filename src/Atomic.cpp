
#include <nstd/Atomic.h>

#if defined(_MSC_VER) && !defined(_WIN64)

#include <Windows.h>

int64 Atomic::increment(volatile int64& var)
{
  return InterlockedIncrement64((__int64 volatile *)&var);
}

uint64 Atomic::increment(volatile uint64& var)
{
  return (uint64)InterlockedIncrement64((__int64 volatile *)&var);
}

int64 Atomic::decrement(volatile int64& var)
{
  return InterlockedDecrement64((__int64 volatile *)&var);
}

uint64 Atomic::decrement(volatile uint64& var)
{
  return (uint64)InterlockedDecrement64((__int64 volatile *)&var);
}

int64 Atomic::compareAndSwap(int64 volatile& var, int64 oldVal, int64 newVal)
{
  return (uint64)InterlockedCompareExchange((unsigned __int64 volatile*)&var, (unsigned __int64)newVal, (unsigned __int64)oldVal);
}

uint64 Atomic::compareAndSwap(uint64 volatile& var, uint64 oldVal, uint64 newVal)
{
  return InterlockedCompareExchange(&var, newVal, oldVal);
}

int64 Atomic::swap(int64 volatile& var, int64 val)
{
  return InterlockedExchange64(&var, val);
}

uint64 Atomic::swap(uint64 volatile& var, uint64 val)
{
  return InterlockedExchange64((int64 volatile*)&var, (int64)val);
}

#endif
