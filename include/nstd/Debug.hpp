
#pragma once

#include <nstd/Base.hpp>

class String;

class Debug
{
public:
  static int print(const char* str);
  static int printf(const char* format, ...);

#ifndef NDEBUG
  static bool getSourceLine(void* addr, String& file, int& line);
#endif
};

#ifdef _MSC_VER
void __cdecl __debugbreak(void);
#define TRAP() __debugbreak()
#else
#if defined(__GNUC__) && defined(_ARM)
__attribute__((gnu_inline, always_inline)) static void __inline__ TRAP(void) {__asm__ volatile("BKPT");}
#else
#define TRAP() __builtin_trap()
#endif
#endif

#if defined(NDEBUG) && !defined(DEBUG)
#undef TRAP
#define TRAP() ((void)0)
#define ASSERT(exp) ((void)1)
#define VERIFY(exp) ((void)(exp))
#else
#ifdef _MSC_VER
#define ASSERT(exp) ((void)(!(exp) && Debug::printf("%s(%u): assertion failed: %s\n", __FILE__, __LINE__, #exp) && (TRAP(), 1)))
#define VERIFY(exp) ((void)(!(exp) && Debug::printf("%s(%u): verification failed: %s\n", __FILE__, __LINE__, #exp) && (TRAP(), 1)))
#else
#define ASSERT(exp) ((void)(!(exp) && Debug::printf("%s:%u: assertion failed: %s\n", __FILE__, __LINE__, #exp) && (TRAP(), 1)))
#define VERIFY(exp) ((void)(!(exp) && Debug::printf("%s:%u: verification failed: %s\n", __FILE__, __LINE__, #exp) && (TRAP(), 1)))
#endif
#endif
