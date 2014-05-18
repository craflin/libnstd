
#pragma once

#include <nstd/Base.h>

#ifdef _MSC_VER
extern "C" long _InterlockedIncrement(long volatile*);
extern "C" long _InterlockedDecrement(long volatile*);
extern "C" long _InterlockedCompareExchange(long volatile*, long, long);
extern "C" void* _InterlockedCompareExchangePointer(void* volatile*, void*, void*);
extern "C" long _InterlockedExchange(long volatile*, long);
extern "C" long _InterlockedAdd(long volatile*, long);
extern "C" long _InterlockedOr(long volatile*, long);
#ifdef _M_AMD64
extern "C" __int64 _InterlockedIncrement64(__int64 volatile*);
extern "C" __int64 _InterlockedDecrement64(__int64 volatile*);
extern "C" __int64 _InterlockedCompareExchange64(__int64 volatile*, __int64, __int64);
extern "C" __int64 _InterlockedExchange64(__int64 volatile*, __int64);
extern "C" __int64 _InterlockedAdd64(__int64 volatile*, __int64);
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

  static inline int32_t compareAndSwap(int32_t volatile& var, int32_t oldVal, int32_t newVal)
  {
#ifdef _MSC_VER
    return (int32_t)_InterlockedCompareExchange((long volatile*)&var, (long)newVal, (long)oldVal);
#else
    return __sync_val_compare_and_swap(var, oldVal, newVal);
#endif
  }

  static inline uint32_t compareAndSwap(uint32_t volatile& var, uint32_t oldVal, uint32_t newVal)
  {
#ifdef _MSC_VER
    return (uint32_t)_InterlockedCompareExchange((long volatile*)&var, (long)newVal, (long)oldVal);
#else
    return __sync_val_compare_and_swap(var, oldVal, newVal);
#endif
  }

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static int64_t compareAndSwap(int64_t volatile& var, int64_t oldVal, int64_t newVal);
#else
  static inline int64_t compareAndSwap(int64_t volatile& var, int64_t oldVal, int64_t newVal)
  {
#ifdef _MSC_VER
    return _InterlockedCompareExchange64((volatile __int64*)&var, newVal, oldVal);
#else
    return __sync_val_compare_and_swap(var, oldVal, newVal);
#endif
  }
#endif

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static uint64_t compareAndSwap(uint64_t volatile& var, uint64_t oldVal, uint64_t newVal);
#else
  static inline uint64_t compareAndSwap(uint64_t volatile& var, uint64_t oldVal, uint64_t newVal)
  {
#ifdef _MSC_VER
    return (uint64_t)_InterlockedCompareExchange64((volatile __int64*)&var, (__int64)newVal, (__int64)oldVal);
#else
    return __sync_val_compare_and_swap(var, oldVal, newVal);
#endif
  }
#endif

  static inline void* compareAndSwap(void* volatile& ptr, void* oldVal, void* newVal)
  {
#ifdef _MSC_VER
    return _InterlockedCompareExchangePointer(&ptr, newVal, oldVal);
#else
    return __sync_val_compare_and_swap(&ptr, oldVal, newVal);
    //void *prev;
    //__asm__ __volatile__ ("lock ; cmpxchg %3,%4"
    //                      : "=a" (prev), "=m" (*ptr)
    //                      : "0" (oldVal), "q" (newVal), "m" (*ptr) : "memory");
    //return prev;
#endif
  }

  template <class T> static inline T* compareAndSwap(T* volatile& ptr, T* oldVal, T* newVal)
  {
#ifdef _MSC_VER
    return (T*)_InterlockedCompareExchangePointer((void* volatile*)&ptr, (void*)newVal, (void*)oldVal);
#else
    return __sync_val_compare_and_swap(&ptr, oldVal, newVal);
    //void *prev;
    //__asm__ __volatile__ ("lock ; cmpxchg %3,%4"
    //                      : "=a" (prev), "=m" (*ptr)
    //                      : "0" (oldVal), "q" (newVal), "m" (*ptr) : "memory");
    //return prev;
#endif
  }

  static inline int32_t swap(int32_t volatile& var,  int32_t val)
  {
#ifdef _MSC_VER
    return (int32_t)_InterlockedExchange((long volatile*)&var, (long)val);
#else
    return __sync_lock_test_and_set(&var, val);
#endif
  }

  static inline uint32_t swap(uint32_t volatile& var,  uint32_t val)
  {
#ifdef _MSC_VER
    return (uint32_t)_InterlockedExchange((long volatile*)&var, (long)val);
#else
    return __sync_lock_test_and_set(&var, val);
#endif
  }

  // todo: swap(int64_t ...
  // todo: swap(uint64_t ...
  // todo: swap(void* ...

  static inline int32_t testAndSet(int32_t volatile& var)
  {
#ifdef _MSC_VER
    return (int32_t)_InterlockedExchange((long volatile*)&var, 1);
#else
    return __sync_lock_test_and_set(&var, 1);
#endif
  }

  // todo: testAndSet uint32_t int64_t ...

  static inline int32_t fetchAndAdd(int32_t volatile& var, int32_t val)
  {
#ifdef _MSC_VER
    return (int32_t)_InterlockedAdd((long volatile*)&var, (long)val);
#else
    return __sync_fetch_and_add(ptr, val);
    //unsigned int result;
    //__asm__ __volatile__ ("lock; xaddl %0, %1" :
    //                      "=r" (result), "=m" (*ptr) : "0" (val), "m" (*ptr)
    //                      : "memory");
    //return result;
#endif
  }

  static inline uint32_t fetchAndAdd(uint32_t volatile& var, uint32_t val)
  {
#ifdef _MSC_VER
    return (uint32_t)_InterlockedAdd((long volatile*)&var, (long)val);
#else
    return __sync_fetch_and_add(ptr, val);
    //unsigned int result;
    //__asm__ __volatile__ ("lock; xaddl %0, %1" :
    //                      "=r" (result), "=m" (*ptr) : "0" (val), "m" (*ptr)
    //                      : "memory");
    //return result;
#endif
  }

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
  */
  static void inline memoryBarrier() 
  {
#ifdef _MSC_VER
    long barrier;
    _InterlockedOr(&barrier, 0);
#else
    __sync_synchronize();
    //__asm__ __volatile__("mfence" : : : "memory"); // SSE2
    // or do something with XCHG // without SSE2
#endif
  }
};
