
#pragma once

#include <nstd/Base.hpp>

class Atomic
{
public:
  static inline int32 increment(volatile int32& var);
  static inline uint32 increment(volatile uint32& var);
  static inline int64 increment(volatile int64& var);
  static inline uint64 increment(volatile uint64& var);

  static inline int32 decrement(volatile int32& var);
  static inline uint32 decrement(volatile uint32& var);
  static inline int64 decrement(volatile int64& var);
  static inline uint64 decrement(volatile uint64& var);

  static inline int32 compareAndSwap(int32 volatile& var, int32 oldVal, int32 newVal);
  static inline uint32 compareAndSwap(uint32 volatile& var, uint32 oldVal, uint32 newVal);
  static inline int64 compareAndSwap(int64 volatile& var, int64 oldVal, int64 newVal);
  static inline uint64 compareAndSwap(uint64 volatile& var, uint64 oldVal, uint64 newVal);
  template <typename T> static inline T* compareAndSwap(T* volatile& ptr, T* oldVal, T* newVal);

  static inline int32 swap(int32 volatile& var, int32 val);
  static inline uint32 swap(uint32 volatile& var, uint32 val);
  static inline int64 swap(int64 volatile& var, int64 val);
  static inline uint64 swap(uint64 volatile& var, uint64 val);
  template <typename T> static inline T* swap(T* volatile& ptr, T* val);

  static inline int32 testAndSet(int32 volatile& var);
  static inline uint32 testAndSet(uint32 volatile& var);
  static inline int64 testAndSet(int64 volatile& var);
  static inline uint64 testAndSet(uint64 volatile& var);

  static inline int32 fetchAndAdd(int32 volatile& var, int32 val);
  static inline uint32 fetchAndAdd(uint32 volatile& var, uint32 val);
  static inline int64 fetchAndAdd(int64 volatile& var, int64 val);
  static inline uint64 fetchAndAdd(uint64 volatile& var, uint64 val);

  static inline void memoryBarrier();

  static inline int32 load(const int32 volatile& var);
  static inline uint32 load(const uint32 volatile& var);
  static inline int64 load(const int64 volatile& var);
  static inline uint64 load(const uint64 volatile& var);
  template <typename T> static inline T* load(T* const volatile& ptr);

  static inline void store(int32 volatile& var, int32 value);
  static inline void store(uint32 volatile& var, uint32 value);
  static inline void store(int64 volatile& var, int64 value);
  static inline void store(uint64 volatile& var, uint64 value);
  template <typename T> static inline void store(T* volatile& ptr, T* value);
};

#ifdef _MSC_VER
extern "C" long _InterlockedIncrement(long volatile*);
extern "C" long _InterlockedDecrement(long volatile*);
extern "C" long _InterlockedCompareExchange(long volatile*, long, long);
extern "C" __int64 _InterlockedCompareExchange64(__int64 volatile*, __int64, __int64);
extern "C" long _InterlockedExchange(long volatile*, long);
extern "C" long _InterlockedExchangeAdd(long volatile*, long);
extern "C" long _InterlockedOr(long volatile*, long);
#ifdef _M_AMD64
extern "C" __int64 _InterlockedIncrement64(__int64 volatile*);
extern "C" __int64 _InterlockedDecrement64(__int64 volatile*);
extern "C" __int64 _InterlockedCompareExchange64(__int64 volatile*, __int64, __int64);
extern "C" __int64 _InterlockedExchange64(__int64 volatile*, __int64);
extern "C" __int64 _InterlockedExchangeAdd64(__int64 volatile*, __int64);
extern "C" __int64 _InterlockedOr64(__int64 volatile*, __int64);
extern "C" void __faststorefence(void);
#endif
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedOr)
#ifdef _M_AMD64
#pragma intrinsic(_InterlockedIncrement64)
#pragma intrinsic(_InterlockedDecrement64)
#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedExchange64)
#pragma intrinsic(_InterlockedExchangeAdd64)
#pragma intrinsic(_InterlockedOr64)
#pragma intrinsic(__faststorefence)
#endif
#endif

#ifdef _MSC_VER

int32 Atomic::increment(volatile int32& var) {return _InterlockedIncrement((long volatile*)&var);}
uint32 Atomic::increment(volatile uint32& var) {return _InterlockedIncrement((long volatile*)&var);}
int32 Atomic::decrement(volatile int32& var) {return _InterlockedDecrement((long volatile*)&var);}
uint32 Atomic::decrement(volatile uint32& var) {return _InterlockedDecrement((long volatile*)&var);}
int32 Atomic::compareAndSwap(int32 volatile& var, int32 oldVal, int32 newVal) {return _InterlockedCompareExchange((long volatile*)&var, newVal, oldVal);}
uint32 Atomic::compareAndSwap(uint32 volatile& var, uint32 oldVal, uint32 newVal) {return _InterlockedCompareExchange((long volatile*)&var, newVal, oldVal);}
int64 Atomic::compareAndSwap(int64 volatile& var, int64 oldVal, int64 newVal) {return _InterlockedCompareExchange64(&var, newVal, oldVal);}
uint64 Atomic::compareAndSwap(uint64 volatile& var, uint64 oldVal, uint64 newVal) {return _InterlockedCompareExchange64((volatile __int64*)&var, newVal, oldVal);}
int32 Atomic::swap(int32 volatile& var, int32 val) {return _InterlockedExchange((long volatile*)&var, val);}
uint32 Atomic::swap(uint32 volatile& var, uint32 val) {return _InterlockedExchange((long volatile*)&var, val);}
int32 Atomic::testAndSet(int32 volatile& var) {return _InterlockedExchange((long volatile*)&var, 1L);}
uint32 Atomic::testAndSet(uint32 volatile& var) {return _InterlockedExchange((long volatile*)&var, 1L);}
int32 Atomic::fetchAndAdd(int32 volatile& var, int32 val) {return _InterlockedExchangeAdd((long volatile*)&var, val);}
uint32 Atomic::fetchAndAdd(uint32 volatile& var, uint32 val) {return _InterlockedExchangeAdd((long volatile*)&var, val);}
#ifndef _M_ARM
int32 Atomic::load(const int32 volatile& var) {return var;}
uint32 Atomic::load(const uint32 volatile& var) {return var;}
void Atomic::store(int32 volatile& var, int32 val) {var = val;}
void Atomic::store(uint32 volatile& var, uint32 val) {var = val;}
#endif
#ifdef _M_AMD64
int64 Atomic::increment(volatile int64& var) {return _InterlockedIncrement64((__int64 volatile*)&var);}
uint64 Atomic::increment(volatile uint64& var) {return _InterlockedIncrement64((__int64 volatile*)&var);}
int64 Atomic::decrement(volatile int64& var) {return _InterlockedDecrement64((__int64 volatile*)&var);}
uint64 Atomic::decrement(volatile uint64& var) {return _InterlockedDecrement64((__int64 volatile*)&var);}
template <typename T> inline T* Atomic::compareAndSwap(T* volatile& ptr, T* oldVal, T* newVal) {return (T*)_InterlockedCompareExchange64((__int64 volatile*)&ptr, (__int64&)newVal, (__int64&)oldVal);}
int64 Atomic::swap(int64 volatile& var, int64 val) {return _InterlockedExchange64(&var, val);}
uint64 Atomic::swap(uint64 volatile& var, uint64 val) {return _InterlockedExchange64((__int64 volatile*)&var, val);}
template <typename T> inline T* Atomic::swap(T* volatile& ptr, T* val) {return (T*)_InterlockedExchange64((__int64 volatile*)&ptr, (__int64&)val);}
int64 Atomic::testAndSet(int64 volatile& var) {return _InterlockedExchange64((__int64 volatile*)&var, 1LL);}
uint64 Atomic::testAndSet(uint64 volatile& var) {return _InterlockedExchange64((__int64 volatile*)&var, 1LL);}
int64 Atomic::fetchAndAdd(int64 volatile& var, int64 val) {return _InterlockedExchangeAdd64(&var, val);}
uint64 Atomic::fetchAndAdd(uint64 volatile& var, uint64 val) {return _InterlockedExchangeAdd64((__int64 volatile*)&var, (__int64)val);}
void Atomic::memoryBarrier() {__faststorefence();}
#ifndef _M_ARM
int64 Atomic::load(const int64 volatile& var) {return var;}
uint64 Atomic::load(const uint64 volatile& var) {return var;}
template <typename T> inline T* Atomic::load(T* const volatile& ptr) {return ptr;}
void Atomic::store(int64 volatile& var, int64 val) {var = val;}
void Atomic::store(uint64 volatile& var, uint64 val) {var = val;}
template <typename T> inline void Atomic::store(T* volatile& ptr, T* val) {ptr = val;}
#endif
#else
int64 Atomic::increment(volatile int64& var) {for(int64 oldVal = var, newVal = oldVal + 1;; newVal = (oldVal = var) + 1) if(_InterlockedCompareExchange64(&var, newVal, oldVal) == oldVal) return newVal;}
uint64 Atomic::increment(volatile uint64& var) {for(uint64 oldVal = var, newVal = oldVal + 1;; newVal = (oldVal = var) + 1) if(_InterlockedCompareExchange64((volatile __int64*)&var, newVal, oldVal) == oldVal) return newVal;}
int64 Atomic::decrement(volatile int64& var) {for(int64 oldVal = var, newVal = oldVal - 1;; newVal = (oldVal = var) - 1) if(_InterlockedCompareExchange64(&var, newVal, oldVal) == oldVal) return newVal;}
uint64 Atomic::decrement(volatile uint64& var) {for(uint64 oldVal = var, newVal = oldVal - 1;; newVal = (oldVal = var) - 1) if(_InterlockedCompareExchange64((volatile __int64*)&var, newVal, oldVal) == oldVal) return newVal;}
template <typename T> inline T* Atomic::compareAndSwap(T* volatile& ptr, T* oldVal, T* newVal) {return (T*)_InterlockedCompareExchange((long volatile*)&ptr, (long&)newVal, (long&)oldVal);}
int64 Atomic::swap(int64 volatile& var, int64 val) {for(int64 oldVal = var;; oldVal = var) if(_InterlockedCompareExchange64(&var, val, oldVal) == oldVal) return oldVal;}
uint64 Atomic::swap(uint64 volatile& var, uint64 val) {for(uint64 oldVal = var;; oldVal = var) if(_InterlockedCompareExchange64((__int64 volatile*)&var, (__int64)val, oldVal) == oldVal) return oldVal;}
template <typename T> inline T* Atomic::swap(T* volatile& ptr, T* val) {return (T*)_InterlockedExchange((long volatile*)&ptr, (long)val);}
int64 Atomic::testAndSet(volatile int64& var) {for(int64 val = var;;) if(_InterlockedCompareExchange64(&var, val, 1LL) == val) return val;}
uint64 Atomic::testAndSet(volatile uint64& var) {for(uint64 val = var;;) if(_InterlockedCompareExchange64((volatile __int64*)&var, val, 1LL) == val) return val;}
inline int64 Atomic::fetchAndAdd(int64 volatile& var, int64 val) {for(int64 oldVal = var;; oldVal = var) if(_InterlockedCompareExchange64(&var, oldVal + val, oldVal) == oldVal) return oldVal;}
inline uint64 Atomic::fetchAndAdd(uint64 volatile& var, uint64 val) {for(uint64 oldVal = var;; oldVal = var) if(_InterlockedCompareExchange64((__int64 volatile*)&var, (__int64)(oldVal + val), oldVal) == oldVal) return oldVal;}
void Atomic::memoryBarrier() {long barrier; _InterlockedOr(&barrier, 0);}
int64 Atomic::load(const volatile int64& var) {for(int64 val = var;; val = var) if(_InterlockedCompareExchange64((volatile __int64*)&var, val, val) == val) return val;}
uint64 Atomic::load(const volatile uint64& var) {for(uint64 val = var;; val = var) if(_InterlockedCompareExchange64((volatile __int64*)&var, val, val) == val) return val;}
template <typename T> inline T* Atomic::load(T* const volatile& ptr) {return (T*)_InterlockedOr((long volatile*)&ptr, 0);}
void Atomic::store(int64 volatile& var, int64 val) {Atomic::swap(var, val);}
void Atomic::store(uint64 volatile& var, uint64 val) {Atomic::swap(var, val);}
template <typename T> inline void Atomic::store(T* volatile& ptr, T* val) {_InterlockedExchange((long volatile*)&ptr, (long)val);}
#endif

#else

int32 Atomic::increment(volatile int32& var) {return __sync_add_and_fetch(&var, 1);}
uint32 Atomic::increment(volatile uint32& var) {return __sync_add_and_fetch(&var, 1);}
int64 Atomic::increment(volatile int64& var) {return __sync_add_and_fetch(&var, 1);}
uint64 Atomic::increment(volatile uint64& var) {return __sync_add_and_fetch(&var, 1);}
int32 Atomic::decrement(volatile int32& var) {return __sync_add_and_fetch(&var, -1);}
uint32 Atomic::decrement(volatile uint32& var) {return __sync_add_and_fetch(&var, -1);}
int64 Atomic::decrement(volatile int64& var) {return __sync_add_and_fetch(&var, -1);}
uint64 Atomic::decrement(volatile uint64& var) {return __sync_add_and_fetch(&var, -1);}
int32 Atomic::compareAndSwap(int32 volatile& var, int32 oldVal, int32 newVal) {return __sync_val_compare_and_swap(&var, oldVal, newVal);}
uint32 Atomic::compareAndSwap(uint32 volatile& var, uint32 oldVal, uint32 newVal) {return __sync_val_compare_and_swap(&var, oldVal, newVal);}
int64 Atomic::compareAndSwap(int64 volatile& var, int64 oldVal, int64 newVal) {return __sync_val_compare_and_swap(&var, oldVal, newVal);}
uint64 Atomic::compareAndSwap(uint64 volatile& var, uint64 oldVal, uint64 newVal) {return __sync_val_compare_and_swap(&var, oldVal, newVal);}
template <typename T> inline T* Atomic::compareAndSwap(T* volatile& ptr, T* oldVal, T* newVal) {return (T*)__sync_val_compare_and_swap(&ptr, oldVal, newVal);}
int32 Atomic::swap(int32 volatile& var, int32 val) {return __sync_lock_test_and_set(&var, val);}
uint32 Atomic::swap(uint32 volatile& var, uint32 val) {return __sync_lock_test_and_set(&var, val);}
int64 Atomic::swap(int64 volatile& var, int64 val) {return __sync_lock_test_and_set(&var, val);}
uint64 Atomic::swap(uint64 volatile& var, uint64 val) {return __sync_lock_test_and_set(&var, val);}
template <typename T> inline T* Atomic::swap(T* volatile& ptr, T* val) {return (T*)__sync_lock_test_and_set(&ptr, val);}
int32 Atomic::testAndSet(int32 volatile& var) {return __sync_lock_test_and_set(&var, 1);}
uint32 Atomic::testAndSet(uint32 volatile& var) {return __sync_lock_test_and_set(&var, 1);}
int64 Atomic::testAndSet(int64 volatile& var) {return __sync_lock_test_and_set(&var, 1);}
uint64 Atomic::testAndSet(uint64 volatile& var) {return __sync_lock_test_and_set(&var, 1);}
int32 Atomic::fetchAndAdd(int32 volatile& var, int32 val) {return __sync_fetch_and_add(&var, val);}
uint32 Atomic::fetchAndAdd(uint32 volatile& var, uint32 val) {return __sync_fetch_and_add(&var, val);}
int64 Atomic::fetchAndAdd(int64 volatile& var, int64 val) {return __sync_fetch_and_add(&var, val);}
uint64 Atomic::fetchAndAdd(uint64 volatile& var, uint64 val) {return __sync_fetch_and_add(&var, val);}
void Atomic::memoryBarrier() {__sync_synchronize();}
int32 Atomic::load(const int32 volatile& var) {__sync_synchronize(); return var;}
uint32 Atomic::load(const uint32 volatile& var) {__sync_synchronize(); return var;}
int64 Atomic::load(const int64 volatile& var) {__sync_synchronize(); return var;}
uint64 Atomic::load(const uint64 volatile& var) {__sync_synchronize(); return var;}
template <typename T> inline T* Atomic::load(T* const volatile& ptr) {__sync_synchronize(); return ptr;}
void Atomic::store(int32 volatile& var, int32 value) {__sync_synchronize(); var = value;}
void Atomic::store(uint32 volatile& var, uint32 value) {__sync_synchronize(); var = value;}
void Atomic::store(int64 volatile& var, int64 value) {__sync_synchronize(); var = value;}
void Atomic::store(uint64 volatile& var, uint64 value) {__sync_synchronize(); var = value;}
template <typename T> inline void Atomic::store(T* volatile& ptr, T* value) {__sync_synchronize(); ptr = value;}

#endif
