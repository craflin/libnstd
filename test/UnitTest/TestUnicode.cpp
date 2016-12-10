
#include <nstd/Debug.h>
#include <nstd/Unicode.h>

void testUnicode()
{
  String str;
  ASSERT(!Unicode::append(0x110000ULL, str));
  for(uint32 i = 0; i < 0x110000ULL; i += 100)
  {
#ifdef _UNICODE
    if(i >= 0xD800 && i <= 0xDFFF)
      continue;
#endif
    str.clear();
    ASSERT(Unicode::append(i, str));
    ASSERT(Unicode::isValid(str));
    ASSERT(Unicode::fromString(str) == i);
  }
}
