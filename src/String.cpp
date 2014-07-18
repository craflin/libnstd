
#include <cstdarg>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstdlib>
#ifdef _MSC_VER
#include <tchar.h>
#endif

#include <nstd/String.h>
#include <nstd/Debug.h>

#ifndef _MSC_VER
#define _tcsstr strstr
#define _tcspbrk strpbrk
#endif

String::EmptyData String::emptyData;
#ifndef _UNICODE
char_t String::lowerCaseMap[0x100] = {0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10 , 11 , 12 , 13 , 14 , 15 , 16 , 17 , 18 , 19 , 20 , 21 , 22 , 23 , 24 , 25 , 26 , 27 , 28 , 29 , 30 , 31 , 32 , 33 , 34 , 35 , 36 , 37 , 38 , 39 , 40 , 41 , 42 , 43 , 44 , 45 , 46 , 47 , 48 , 49 , 50 , 51 , 52 , 53 , 54 , 55 , 56 , 57 , 58 , 59 , 60 , 61 , 62 , 63 , 64 , 97 , 98 , 99 , 100 , 101 , 102 , 103 , 104 , 105 , 106 , 107 , 108 , 109 , 110 , 111 , 112 , 113 , 114 , 115 , 116 , 117 , 118 , 119 , 120 , 121 , 122 , 91 , 92 , 93 , 94 , 95 , 96 , 97 , 98 , 99 , 100 , 101 , 102 , 103 , 104 , 105 , 106 , 107 , 108 , 109 , 110 , 111 , 112 , 113 , 114 , 115 , 116 , 117 , 118 , 119 , 120 , 121 , 122 , 123 , 124 , 125 , 126 , 127 , -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -104, -103, -102, -101, -100, -99 , -98 , -97 , -96 , -95 , -94 , -93 , -92 , -91 , -90 , -89 , -88 , -87 , -86 , -85 , -84 , -83 , -82 , -81 , -80 , -79 , -78 , -77 , -76 , -75 , -74 , -73 , -72 , -71 , -70 , -69 , -68 , -67 , -66 , -65 , -64 , -63 , -62 , -61 , -60 , -59 , -58 , -57 , -56 , -55 , -54 , -53 , -52 , -51 , -50 , -49 , -48 , -47 , -46 , -45 , -44 , -43 , -42 , -41 , -40 , -39 , -38 , -37 , -36 , -35 , -34 , -33 , -32 , -31 , -30 , -29 , -28 , -27 , -26 , -25 , -24 , -23 , -22 , -21 , -20 , -19 , -18 , -17 , -16 , -15 , -14 , -13 , -12 , -11 , -10 , -9 , -8 , -7 , -6 , -5 , -4 , -3 , -2 , -1};
char_t String::upperCaseMap[0x100] = {0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10 , 11 , 12 , 13 , 14 , 15 , 16 , 17 , 18 , 19 , 20 , 21 , 22 , 23 , 24 , 25 , 26 , 27 , 28 , 29 , 30 , 31 , 32 , 33 , 34 , 35 , 36 , 37 , 38 , 39 , 40 , 41 , 42 , 43 , 44 , 45 , 46 , 47 , 48 , 49 , 50 , 51 , 52 , 53 , 54 , 55 , 56 , 57 , 58 , 59 , 60 , 61 , 62 , 63 , 64 , 65 , 66 , 67 , 68 , 69 , 70 , 71 , 72 , 73 , 74 , 75 , 76 , 77 , 78 , 79 , 80 , 81 , 82 , 83 , 84 , 85 , 86 , 87 , 88 , 89 , 90 , 91 , 92 , 93 , 94 , 95 , 96 , 65 , 66 , 67 , 68 , 69 , 70 , 71 , 72 , 73 , 74 , 75 , 76 , 77 , 78 , 79 , 80 , 81 , 82 , 83 , 84 , 85 , 86 , 87 , 88 , 89 , 90 , 123 , 124 , 125 , 126 , 127 , -128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113, -112, -111, -110, -109, -108, -107, -106, -105, -104, -103, -102, -101, -100, -99 , -98 , -97 , -96 , -95 , -94 , -93 , -92 , -91 , -90 , -89 , -88 , -87 , -86 , -85 , -84 , -83 , -82 , -81 , -80 , -79 , -78 , -77 , -76 , -75 , -74 , -73 , -72 , -71 , -70 , -69 , -68 , -67 , -66 , -65 , -64 , -63 , -62 , -61 , -60 , -59 , -58 , -57 , -56 , -55 , -54 , -53 , -52 , -51 , -50 , -49 , -48 , -47 , -46 , -45 , -44 , -43 , -42 , -41 , -40 , -39 , -38 , -37 , -36 , -35 , -34 , -33 , -32 , -31 , -30 , -29 , -28 , -27 , -26 , -25 , -24 , -23 , -22 , -21 , -20 , -19 , -18 , -17 , -16 , -15 , -14 , -13 , -12 , -11 , -10 , -9 , -8 , -7 , -6 , -5 , -4 , -3 , -2 , -1};
#endif

void_t String::detach(size_t copyLength, size_t minCapacity)
{
  ASSERT(copyLength <= minCapacity);

  if(data->ref == 1)
  {
    size_t currentCapacity = (Memory::size(data) - sizeof(Data)) / sizeof(tchar_t) - 1;
    if(minCapacity <= currentCapacity /*&& currentCapacity - minCapacity < Memory::pageSize()*/)
    {
      data->len = copyLength;
      ((tchar_t*)data->str)[copyLength] = _T('\0');
      return;
    }
  }

  Data* newData = (Data*)Memory::alloc((minCapacity + 1) * sizeof(tchar_t) + sizeof(Data));
  newData->str = (tchar_t*)((byte_t*)newData + sizeof(Data));
  if(data->len > 0)
  {
    Memory::copy((tchar_t*)newData->str, data->str, (data->len < copyLength ? data->len : copyLength) * sizeof(tchar_t));
    ((tchar_t*)newData->str)[copyLength] = _T('\0');
  }
  else
    *(tchar_t*)newData->str = _T('\0');
  newData->len = copyLength;
  newData->ref = 1;

  if(data->ref && Atomic::decrement(data->ref) == 0)
    Memory::free(data);

  data = newData;
}

int_t String::printf(const tchar_t* format, ...)
{
  detach(0, 200);

  int_t result;
  va_list ap;
  va_start(ap, format);

  {
    size_t capacity = Memory::size(data) - sizeof(Data);
#ifdef _UNICODE
    result = _vsnwprintf((wchar_t*)data->str, capacity, format, ap);
#else
    result = vsnprintf((char_t*)data->str, capacity, format, ap);
#endif
    if(result >= 0 && result < (int_t)capacity)
    {
      data->len = result;
      va_end(ap);
      return result;
    }
  }

  // buffer was too small: compute size, reserve buffer, print again
  {
#ifdef _MSC_VER
#ifdef _UNICODE
    result = _vscwprintf(format, ap);
#else
    result = _vscprintf(format, ap);
#endif
#else
    result = vsnprintf(0, 0, format, ap);
#endif
    ASSERT(result >= 0);
    if(result < 0)
      return -1;
    reserve(result);
#ifdef _UNICODE
    result = _vsnwprintf((wchar_t*)data->str, result + 1, format, ap);
#else
    result = vsnprintf((char_t*)data->str, result + 1, format, ap);
#endif
    ASSERT(result >= 0);
    data->len = result;
    va_end(ap);
    return result;
  }
}

#if defined(_MSC_VER) && _MSC_VER <= 1600
#ifdef _UNICODE
static int vswscanf(const wchar_t* s, const wchar_t* fmt, va_list ap)
#else
static int vsscanf(const char* s, const char* fmt, va_list ap)
#endif
{
  struct Args
  {
    void *a[32];
  } args;
  for (int i = 0; i < sizeof(args.a) / sizeof(void*); ++i)
    args.a[i] = va_arg(ap, void*);
#ifdef _UNICODE
  return swscanf(s, fmt, args);
#else
  return sscanf(s, fmt, args);
#endif
}
#ifdef _UNICODE
#define _wtoll _wtoi64
#define wcstoull _wcstoui64
#else
#define atoll _atoi64
#define strtoull _strtoui64
#endif
#endif

int_t String::scanf(const tchar_t* format, ...) const
{
  int_t result;
  va_list ap;
  va_start(ap, format);
#ifdef _UNICODE
  result = vswscanf(data->str, format, ap);
#else
  result = vsscanf(data->str, format, ap);
#endif
  va_end(ap);
  return result;
}

#ifdef _UNICODE
int_t String::toInt() const {return _wtoi(data->str);}
uint_t String::toUInt() const {return wcstoul(data->str, 0, 10);}
int64_t String::toInt64() const {return _wtoll(data->str);}
uint64_t String::toUInt64() const {return wcstoull(data->str, 0, 10);}
double String::toDouble() const {return _wtof(data->str);}
#else
int_t String::toInt() const {return atoi(data->str);}
uint_t String::toUInt() const {return strtoul(data->str, 0, 10);}
int64_t String::toInt64() const {return atoll(data->str);}
uint64_t String::toUInt64() const {return strtoull(data->str, 0, 10);}
double String::toDouble() const {return atof(data->str);}
#endif

const tchar_t* String::find(const tchar_t* str) const {return _tcsstr(data->str, str);}
const tchar_t* String::findOneOf(const tchar_t* chars) const {return _tcspbrk(data->str, chars);}
const tchar_t* String::findLast(const tchar_t* str) const {return String::findLast(data->str, str);}
const tchar_t* String::findLastOf(const tchar_t* chars) const {return String::findLastOf(data->str, chars);}

#ifdef _UNICODE
String& String::toLowerCase()
{
  detach(data->len, data->len);
  for(wchar_t* str = (wchar_t*)data->str; *str; ++str)
    *str = towlower(*str);
  return *this;
}

String& String::toUpperCase()
{
  detach(data->len, data->len);
  for(wchar_t* str = (wchar_t*)data->str; *str; ++str)
    *str = towupper(*str);
  return *this;
}
#endif

#ifdef _UNICODE
wchar_t String::toLowerCase(wchar_t c) {return towlower(c);}
wchar_t String::toUpperCase(wchar_t c) {return towupper(c);}
bool_t String::isSpace(wchar_t c) {return iswspace(c) != 0;}
#endif

#ifdef _UNICODE
bool_t String::isAlnum(wchar_t c) { return iswalnum(c) != 0; };
bool_t String::isAlpha(wchar_t c) { return iswalpha(c) != 0; };
bool_t String::isDigit(wchar_t c) { return iswdigit(c) != 0; };
bool_t String::isLower(wchar_t c) { return iswlower(c) != 0; };
bool_t String::isPrint(wchar_t c) { return iswprint(c) != 0; };
bool_t String::isPunct(wchar_t c) { return iswpunct(c) != 0; };
bool_t String::isUpper(wchar_t c) { return iswupper(c) != 0; };
bool_t String::isXDigit(wchar_t c) { return iswxdigit(c) != 0; };
#else
bool_t String::isAlnum(char_t c) { return isalnum((uchar_t&)c) != 0; };
bool_t String::isAlpha(char_t c) {return isalpha((uchar_t&)c) != 0;};
bool_t String::isDigit(char_t c) { return isdigit((uchar_t&)c) != 0; };
bool_t String::isLower(char_t c) { return islower((uchar_t&)c) != 0; };
bool_t String::isPrint(char_t c) { return isprint((uchar_t&)c) != 0; };
bool_t String::isPunct(char_t c) { return ispunct((uchar_t&)c) != 0; };
bool_t String::isUpper(char_t c) { return isupper((uchar_t&)c) != 0; };
bool_t String::isXDigit(char_t c) { return isxdigit((uchar_t&)c) != 0; };
#endif

const tchar_t* String::find(const tchar_t* in, const tchar_t* str) {return _tcsstr(in, str);}
const tchar_t* String::findOneOf(const tchar_t* in, const tchar_t* chars) {return _tcspbrk(in, chars);}
const tchar_t* String::findLast(const tchar_t* in, const tchar_t* str)
{
  const tchar_t* result = 0;
  const tchar_t* match = _tcsstr(in, str);
  for(;;)
  {
    if(!match)
      return result;
    result = match;
    match = _tcsstr(match + 1, str);
  }
}
const tchar_t* String::findLastOf(const tchar_t* in, const tchar_t* chars)
{
  const tchar_t* result = 0;
  const tchar_t* match = _tcspbrk(in, chars);
  for(;;)
  {
    if(!match)
      return result;
    result = match;
    match = _tcspbrk(match + 1, chars);
  }
}

String String::fromInt(int_t value)
{
  String result;
  result.printf(_T("%d"), value);
  return result;
}

String String::fromUInt(uint_t value)
{
  String result;
  result.printf(_T("%u"), value);
  return result;
}

String String::fromInt64(int64_t value)
{
  String result;
  result.printf(_T("%lld"), value);
  return result;
}

String String::fromUInt64(uint64_t value)
{
  String result;
  result.printf(_T("%llu"), value);
  return result;
}

String String::fromDouble(double value)
{
  String result;
  result.printf(_T("%f"), value);
  return result;
}
