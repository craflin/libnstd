
#pragma once

#include <nstd/Base.h>

class Debug
{
public:
  static int_t print(const char_t* str);
  static int_t printf(const char_t* format, ...);
};

#ifdef _MSC_VER
void __cdecl __debugbreak(void);
#define HALT() __debugbreak()
#else
#define HALT() __builtin_trap()
#endif

#ifdef NDEBUG
#undef HALT
#define HALT() ((void_t)0)
#define ASSERT(exp) ((void_t)1)
#define VERIFY(exp) ((void_t)(exp))
#else
#define ASSERT(exp) ((void_t)(!(exp) && Debug::printf("%s:%u: assertion failed: %s\n", __FILE__, __LINE__, #exp) && (HALT(), 1)))
#define VERIFY(exp) ((void_t)(!(exp) && Debug::printf("%s:%u: verification failed: %s\n", __FILE__, __LINE__, #exp) && (HALT(), 1)))
#endif
