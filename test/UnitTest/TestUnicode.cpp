
#include <nstd/Debug.h>
#include <nstd/Unicode.h>

void_t testUnicode()
{
  if (sizeof(tchar_t) != 1)
    return;
  uint32_t ch = 0x5d8;
  String encoded = Unicode::toString(ch);
  ASSERT(Unicode::length(*(const tchar_t*)encoded) > 1);
  ASSERT(Unicode::isValid(encoded));
  ASSERT(!Unicode::isValid(String()));
  ASSERT(Unicode::fromString(encoded) == ch);
}
