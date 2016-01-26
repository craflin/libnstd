
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
#define _tcschr strchr
#define _tcsstr strstr
#define _tcspbrk strpbrk
#endif

String::EmptyData String::emptyData;
#ifndef _UNICODE
char_t String::lowerCaseMap[0x101] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x40\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x5b\x5c\x5d\x5e\x5f\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";
char_t String::upperCaseMap[0x101] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f\x60\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x7b\x7c\x7d\x7e\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";
#endif

int_t String::printf(const tchar_t* format, ...)
{
  detach(0, 200);

  int_t result;
  va_list ap;
  va_start(ap, format);

  {
#ifdef _UNICODE
    result = _vsnwprintf((wchar_t*)data->str, data->capacity, format, ap);
#else
    result = vsnprintf((char_t*)data->str, data->capacity, format, ap);
#endif
    if(result >= 0 && (size_t)result < data->capacity)
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
    detach(0, result);
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

const tchar_t* String::find(tchar_t c, size_t start) const {return start >= data->len ? 0 : _tcschr(data->str + start, c);}
const tchar_t* String::find(const tchar_t* str) const {return _tcsstr(data->str, str);}
const tchar_t* String::find(const tchar_t* str, size_t start) const {return start >= data->len ? 0 : _tcsstr(data->str + start, str);}
const tchar_t* String::findOneOf(const tchar_t* chars) const {return _tcspbrk(data->str, chars);}
const tchar_t* String::findOneOf(const tchar_t* chars, size_t start) const {return start >= data->len ? 0 : _tcspbrk(data->str + start, chars);}
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
int_t String::toInt(const tchar_t* s) {return _wtoi(s);}
uint_t String::toUInt(const tchar_t* s) {return wcstoul(s, 0, 10);}
int64_t String::toInt64(const tchar_t* s) {return _wtoll(s);}
uint64_t String::toUInt64(const tchar_t* s) {return wcstoull(s, 0, 10);}
double String::toDouble(const tchar_t* s) {return _wtof(s);}
#else
int_t String::toInt(const tchar_t* s) {return atoi(s);}
uint_t String::toUInt(const tchar_t* s) {return strtoul(s, 0, 10);}
int64_t String::toInt64(const tchar_t* s) {return atoll(s);}
uint64_t String::toUInt64(const tchar_t* s) {return strtoull(s, 0, 10);}
double String::toDouble(const tchar_t* s) {return atof(s);}
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

String String::token(char_t separator, size_t& start) const
{
  const char_t* endStr = find(separator, start);
  if(endStr)
  {
    size_t len = endStr - (data->str + start);
    String result = String::substr(start, len);
    start += len + 1;
    return result;
  }
  String result = substr(start);
  start = data->len;
  return result;
}

String String::token(const char_t* separators, size_t& start) const
{
  const char_t* endStr = findOneOf(separators, start);
  if(endStr)
  {
    size_t len = endStr - (data->str + start);
    String result = substr(start, len);
    start += len + 1;
    return result;
  }
  String result = substr(start);
  start = data->len;
  return result;
}
