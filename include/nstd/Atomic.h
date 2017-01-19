
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

  static inline int32 increment(volatile int32& var)
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
    int result;
    int* pValInt = (int*)&var;

    asm volatile( 
        "lock; xaddl %%eax, %2;"
        :"=a" (result) 
        : "a" (1), "m" (*pValInt) 
        :"memory" );

    return result + 1;
    */
#endif
  }

  static inline uint32 increment(volatile uint32& var)
  {
#ifdef _MSC_VER
    return _InterlockedIncrement((long volatile *)&var);
#else
    return __sync_add_and_fetch(&var, 1); // untested
#endif
  }

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static int64 increment(volatile int64& var);
#else
  static inline int64 increment(volatile int64& var)
  {
#ifdef _MSC_VER
    return _InterlockedIncrement64((__int64 volatile *)&var);
#else
    return __sync_add_and_fetch(&var, 1); // untested
#endif
  }
#endif

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static uint64 increment(volatile uint64& var);
#else
  static inline uint64 increment(volatile uint64& var)
  {
#ifdef _MSC_VER
    return _InterlockedIncrement64((__int64 volatile *)&var);
#else
    return __sync_add_and_fetch(&var, 1); // untested
#endif
  }
#endif


  static inline int32 decrement(volatile int32& var)
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

  static inline uint32 decrement(volatile uint32& var)
  {
#ifdef _MSC_VER
    return _InterlockedDecrement((long volatile *)&var);
#else
    return __sync_add_and_fetch(&var, -1); // untested
#endif
  }

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static int64 decrement(volatile int64& var);
#else
  static inline int64 decrement(volatile int64& var)
  {
#ifdef _MSC_VER
    return _InterlockedDecrement64((__int64 volatile *)&var);
#else
    return __sync_add_and_fetch(&var, -1); // untested
#endif
  }
#endif

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static uint64 decrement(volatile uint64& var);
#else
  static inline uint64 decrement(volatile uint64& var)
  {
#ifdef _MSC_VER
    return _InterlockedDecrement64((__int64 volatile *)&var);
#else
    return __sync_add_and_fetch(&var, -1); // untested
#endif
  }
#endif

  static inline int32 compareAndSwap(int32 volatile& var, int32 oldVal, int32 newVal)
  {
#ifdef _MSC_VER
    return (int32)_InterlockedCompareExchange((long volatile*)&var, (long)newVal, (long)oldVal);
#else
    return __sync_val_compare_and_swap(&var, oldVal, newVal);
#endif
  }

  static inline uint32 compareAndSwap(uint32 volatile& var, uint32 oldVal, uint32 newVal)
  {
#ifdef _MSC_VER
    return (uint32)_InterlockedCompareExchange((long volatile*)&var, (long)newVal, (long)oldVal);
#else
    return __sync_val_compare_and_swap(&var, oldVal, newVal);
#endif
  }

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static int64 compareAndSwap(int64 volatile& var, int64 oldVal, int64 newVal);
#else
  static inline int64 compareAndSwap(int64 volatile& var, int64 oldVal, int64 newVal)
  {
#ifdef _MSC_VER
    return _InterlockedCompareExchange64((volatile __int64*)&var, newVal, oldVal);
#else
    return __sync_val_compare_and_swap(&var, oldVal, newVal);
#endif
  }
#endif

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static uint64 compareAndSwap(uint64 volatile& var, uint64 oldVal, uint64 newVal);
#else
  static inline uint64 compareAndSwap(uint64 volatile& var, uint64 oldVal, uint64 newVal)
  {
#ifdef _MSC_VER
    return (uint64)_InterlockedCompareExchange64((volatile __int64*)&var, (__int64)newVal, (__int64)oldVal);
#else
    return __sync_val_compare_and_swap(&var, oldVal, newVal);
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

  static inline int32 swap(int32 volatile& var, int32 val)
  {
#ifdef _MSC_VER
    return (int32)_InterlockedExchange((long volatile*)&var, (long)val);
#else
    return __sync_lock_test_and_set(&var, val);
#endif
  }

  static inline uint32 swap(uint32 volatile& var, uint32 val)
  {
#ifdef _MSC_VER
    return (uint32)_InterlockedExchange((long volatile*)&var, (long)val);
#else
    return __sync_lock_test_and_set(&var, val);
#endif
  }

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static int64 swap(int64 volatile& var, int64 val);
#else
  static inline int64 swap(int64 volatile& var, int64 val)
  {
#ifdef _MSC_VER
    return (uint64)_InterlockedExchange64((long long volatile*)&var, (long long)val);
#else
    return __sync_lock_test_and_set(&var, val);
#endif
  }
#endif

#if defined(_MSC_VER) && !defined(_M_AMD64)
  static uint64 swap(uint64 volatile& var, uint64 val);
#else
  static inline uint64 swap(uint64 volatile& var, uint64 val)
  {
#ifdef _MSC_VER
    return (uint64)_InterlockedExchange64((long long volatile*)&var, (long long)val);
#else
    return __sync_lock_test_and_set(&var, val);
#endif
  }
#endif

  static inline void* swap(void* volatile& ptr, void* val)
  {
#ifdef _MSC_VER
#if defined(_M_AMD64)
    return (void*)_InterlockedExchange64((long long volatile*)&ptr, (long long)val);
#else
    return (void*)_InterlockedExchange((long volatile*)&ptr, (long)val);
#endif
#else
    return __sync_lock_test_and_set(&ptr, val);
#endif
  }

  template <class T> static inline T* swap(T* volatile& ptr, T* val)
  {
#ifdef _MSC_VER
#if defined(_M_AMD64)
    return (T*)_InterlockedExchange64((long long volatile*)&ptr, (long long)val);
#else
    return (T*)_InterlockedExchange((long volatile*)&ptr, (long)val);
#endif
#else
    return __sync_lock_test_and_set(&ptr, val);
#endif
  }

  static inline int32 testAndSet(int32 volatile& var)
  {
#ifdef _MSC_VER
    return (int32)_InterlockedExchange((long volatile*)&var, 1);
#else
    return __sync_lock_test_and_set(&var, 1);
#endif
  }

  // todo: testAndSet uint32 int64 ...

  static inline int32 fetchAndAdd(int32 volatile& var, int32 val)
  {
#ifdef _MSC_VER
    return (int32)_InterlockedAdd((long volatile*)&var, (long)val);
#else
    return __sync_fetch_and_add(&var, val);
    //unsigned int result;
    //__asm__ __volatile__ ("lock; xaddl %0, %1" :
    //                      "=r" (result), "=m" (*ptr) : "0" (val), "m" (*ptr)
    //                      : "memory");
    //return result;
#endif
  }

  static inline uint32 fetchAndAdd(uint32 volatile& var, uint32 val)
  {
#ifdef _MSC_VER
    return (uint32)_InterlockedAdd((long volatile*)&var, (long)val);
#else
    return __sync_fetch_and_add(&var, val);
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
