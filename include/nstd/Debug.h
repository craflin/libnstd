
#pragma once

#include <nstd/Base.h>

class Debug
{
public:
  static int_t print(const tchar_t* str);
  static int_t printf(const tchar_t* format, ...);
};

#ifdef _MSC_VER
void __cdecl __debugbreak(void);
#define TRAP() __debugbreak()
#else
#if defined(__GNUC__) && defined(_ARM)
__attribute__((gnu_inline, always_inline)) static void __inline__ TRAP(void) {__asm__ volatile(".inst 0xe7f001f0");}
#else
#define TRAP() __builtin_trap()
#endif
#endif

#ifdef NDEBUG
#undef TRAP
#define TRAP() ((void_t)0)
#define ASSERT(exp) ((void_t)1)
#define VERIFY(exp) ((void_t)(exp))
#else
#ifdef _MSC_VER
#define ASSERT(exp) ((void_t)(!(exp) && Debug::printf(_T("%s(%u): assertion failed: %s\n"), __TFILE__, __LINE__, _T(#exp)) && (TRAP(), 1)))
#define VERIFY(exp) ((void_t)(!(exp) && Debug::printf(_T("%s(%u): verification failed: %s\n"), __TFILE__, __LINE__, _T(#exp)) && (TRAP(), 1)))
#else
#define ASSERT(exp) ((void_t)(!(exp) && Debug::printf(_T("%s:%u: assertion failed: %s\n"), __TFILE__, __LINE__, #exp) && (TRAP(), 1)))
#define VERIFY(exp) ((void_t)(!(exp) && Debug::printf(_T("%s:%u: verification failed: %s\n"), __TFILE__, __LINE__, #exp) && (TRAP(), 1)))
#endif
#endif
