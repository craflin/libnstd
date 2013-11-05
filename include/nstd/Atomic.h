
#pragma once

#include <nstd/Base.h>

#ifdef _MSC_VER
extern "C" long _InterlockedIncrement(long volatile *Addend);
extern "C" long _InterlockedDecrement(long volatile *Addend);
#ifdef _M_AMD64
extern "C" __int64 _InterlockedIncrement64(__int64 volatile *Addend);
extern "C" __int64 _InterlockedDecrement64(__int64 volatile *Addend);
#endif
#endif

class Atomic
{
public:

  static inline int32_t increment(volatile int32_t& var)
  {
#ifdef _MSC_VER
    return _InterlockedIncrement((long volatile *)&var);
    /*
    unsigned int result;
    __asm
    {
      mov ecx, dword ptr [var]
      mov eax, 1
      lock xadd dword ptr [ecx], eax
      inc eax
      mov result, eax
    }
    return result;
    */
#else
    return __sync_add_and_fetch(&var, 1); // untested
    /*
    int_t result;
    int_t* pValInt = (int_t*)&var;

    asm volatile( 
        "lock; xaddl %%eax, %2;"
        :"=a" (result) 
        : "a" (1), "m" (*pValInt) 
        :"memory" );

    return result + 1;
    */
#endif
  }

  static inline uint32_t increment(volatile uint32_t& var)
  {
#ifdef _MSC_VER
    return _InterlockedIncrement((long volatile *)&var);
#else
    return __sync_add_and_fetch(&var, 1); // untested
#endif
  }

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static int64_t increment(volatile int64_t& var);
#else
  static inline int64_t increment(volatile int64_t& var)
  {
#ifdef _MSC_VER
    return _InterlockedIncrement64((__int64 volatile *)&var);
#else
    return __sync_add_and_fetch(&var, 1); // untested
#endif
  }
#endif

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static uint64_t increment(volatile uint64_t& var);
#else
  static inline uint64_t increment(volatile uint64_t& var)
  {
#ifdef _MSC_VER
    return _InterlockedIncrement64((__int64 volatile *)&var);
#else
    return __sync_add_and_fetch(&var, 1); // untested
#endif
  }
#endif


  static inline int32_t decrement(volatile int32_t& var)
  {
#ifdef _MSC_VER
    return _InterlockedDecrement((long volatile *)&var);
    /*
    unsigned int result;
    __asm
    {
      mov ecx, dword ptr [var]
      mov eax, -1
      lock xadd dword ptr [ecx], eax
      dec eax
      mov result, eax
    }
    return result;
    */
#else
    return __sync_add_and_fetch(&var, -1); // untested
#endif
  }

  static inline uint32_t decrement(volatile uint32_t& var)
  {
#ifdef _MSC_VER
    return _InterlockedDecrement((long volatile *)&var);
#else
    return __sync_add_and_fetch(&var, -1); // untested
#endif
  }

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static int64_t decrement(volatile int64_t& var);
#else
  static inline int64_t decrement(volatile int64_t& var)
  {
#ifdef _MSC_VER
    return _InterlockedDecrement64((__int64 volatile *)&var);
#else
    return __sync_add_and_fetch(&var, -1); // untested
#endif
  }
#endif

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static uint64_t decrement(volatile uint64_t& var);
#else
  static inline uint64_t decrement(volatile uint64_t& var)
  {
#ifdef _MSC_VER
    return _InterlockedDecrement64((__int64 volatile *)&var);
#else
    return __sync_add_and_fetch(&var, -1); // untested
#endif
  }
#endif

  /*
  static inline void* swapPtr(void* volatile* ptr, void *val)
  {
#ifdef _MSC_VER
    return InterlockedExchangePointer(ptr, val);
#else
    return __sync_lock_test_and_set(ptr, val);
    //__asm__ __volatile__ ("xchg %0,%1"
    //                      :"=r" (val), "=m" (*ptr)
    //                      : "0" (val),  "m" (*ptr) : "memory");
    //return val;
#endif
  }

  static inline int swapInt(int volatile* ptr,  int val)
  {
#ifdef _MSC_VER
    return InterlockedExchange((long volatile*)ptr, (long)val);
#else
    return __sync_lock_test_and_set(ptr, val);
#endif
  }

  static inline int testAndSetInt(int volatile* ptr)
  {
#ifdef _MSC_VER
    return InterlockedExchange((long volatile*)ptr, 1);
#else
    return __sync_lock_test_and_set(ptr, 1);
#endif
  }

  static inline int fetchAndAddInt(int volatile* ptr, int val)
  {
#ifdef _MSC_VER
    return InterlockedAdd((long volatile*)ptr, (long)val);
#else
    return __sync_fetch_and_add(ptr, val);
    //unsigned int result;
    //__asm__ __volatile__ ("lock; xaddl %0, %1" :
    //                      "=r" (result), "=m" (*ptr) : "0" (val), "m" (*ptr)
    //                      : "memory");
    //return result;
#endif
  }

  static inline void* compareAndSwapPtr(void* volatile* ptr, void* oldVal, void* newVal)
  {
#ifdef _MSC_VER
    return InterlockedCompareExchangePointer(ptr, newVal, oldVal);
#else
    return __sync_val_compare_and_swap(ptr, oldVal, newVal);
    //void *prev;
    //__asm__ __volatile__ ("lock ; cmpxchg %3,%4"
    //                      : "=a" (prev), "=m" (*ptr)
    //                      : "0" (oldVal), "q" (newVal), "m" (*ptr) : "memory");
    //return prev;
#endif
  }

  static inline int compareAndSwapInt(int volatile* ptr, int oldVal, int newVal)
  {
#ifdef _MSC_VER
    return InterlockedCompareExchange((long volatile*)ptr, (long)newVal, (long)oldVal);
#else
    return __sync_val_compare_and_swap(ptr, oldVal, newVal);
#endif
  }

  static void inline memoryBarrier() 
  {
#ifdef _MSC_VER
    MemoryBarrier();
#else
    __sync_synchronize();
    //__asm__ __volatile__("mfence" : : : "memory"); // SSE2
    // or do something with XCHG // without SSE2
#endif
  }
  */
};
