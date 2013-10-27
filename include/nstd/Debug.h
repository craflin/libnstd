
#pragma once

#include <nstd/Base.h>

class Debug
{
public:
  static int_t print(const char* str);
  static int_t printf(const char_t* format, ...);
};

#if defined(_MSC_VER)
void __cdecl __debugbreak(void);
#endif

#ifdef NDEBUG
#define HALT() ((void_t)0)
#define ASSERT(exp) ((void_t)1)
#define VERIFY(exp) ((void_t)(exp))
#else
#define HALT() __debugbreak()
#define ASSERT(exp) ((void_t)(!(exp) && Debug::printf("%s:%u: assertion failed: %s\n", __FILE__, __LINE__, #exp) && (__debugbreak(), 1)))
#define VERIFY(exp) ((void_t)(!(exp) && Debug::printf("%s:%u: verification failed: %s\n", __FILE__, __LINE__, #exp) && (__debugbreak(), 1)))
#endif
