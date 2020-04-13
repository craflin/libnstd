
#include <cstdarg>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstdlib>
#ifdef _MSC_VER
#include <tchar.h>
#endif

#include <nstd/String.hpp>
#include <nstd/Debug.hpp>
#include <nstd/List.hpp>
#include <nstd/HashSet.hpp>

#ifndef _MSC_VER
#define _tcschr strchr
#define _tcsstr strstr
#define _tcspbrk strpbrk
#endif

String::EmptyData String::emptyData;
#ifndef _UNICODE
char String::lowerCaseMap[0x101] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x40\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x5b\x5c\x5d\x5e\x5f\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";
char String::upperCaseMap[0x101] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f\x60\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x7b\x7c\x7d\x7e\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";
#endif

int String::printf(const tchar* format, ...)
{
  detach(0, 200);

  int result;
  va_list ap;
  va_start(ap, format);

  {
#ifdef _UNICODE
    result = _vsnwprintf((wchar_t*)data->str, data->capacity, format, ap);
#else
    result = vsnprintf((char*)data->str, data->capacity, format, ap);
#endif
    if(result >= 0 && (usize)result < data->capacity)
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
    result = vsnprintf((char*)data->str, result + 1, format, ap);
#endif
    ASSERT(result >= 0);
    data->len = result;
    va_end(ap);
    return result;
  }
}

String String::fromPrintf(const tchar* format, ...)
{
  String s(200);

  int result;
  va_list ap;
  va_start(ap, format);

  {
#ifdef _UNICODE
    result = _vsnwprintf((wchar_t*)s.data->str, s.data->capacity, format, ap);
#else
    result = vsnprintf((char*)s.data->str, s.data->capacity, format, ap);
#endif
    if(result >= 0 && (usize)result < s.data->capacity)
    {
      s.data->len = result;
      va_end(ap);
      return s;
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
      return String();
    s.detach(0, result);
#ifdef _UNICODE
    result = _vsnwprintf((wchar_t*)s.data->str, result + 1, format, ap);
#else
    result = vsnprintf((char*)s.data->str, result + 1, format, ap);
#endif
    ASSERT(result >= 0);
    s.data->len = result;
    va_end(ap);
    return s;
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

int String::scanf(const tchar* format, ...) const
{
  int result;
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
int String::toInt() const {return _wtoi(data->str);}
uint String::toUInt() const {return wcstoul(data->str, 0, 10);}
int64 String::toInt64() const {return _wtoll(data->str);}
uint64 String::toUInt64() const {return wcstoull(data->str, 0, 10);}
double String::toDouble() const {return _wtof(data->str);}
#else
int String::toInt() const {return atoi(data->str);}
uint String::toUInt() const {return strtoul(data->str, 0, 10);}
int64 String::toInt64() const {return atoll(data->str);}
uint64 String::toUInt64() const {return strtoull(data->str, 0, 10);}
double String::toDouble() const {return atof(data->str);}
#endif

const tchar* String::find(tchar c, usize start) const {return start >= data->len ? 0 : _tcschr(data->str + start, c);}
const tchar* String::find(const tchar* str) const {return _tcsstr(data->str, str);}
const tchar* String::find(const tchar* str, usize start) const {return start >= data->len ? 0 : _tcsstr(data->str + start, str);}
const tchar* String::findOneOf(const tchar* chars) const {return _tcspbrk(data->str, chars);}
const tchar* String::findOneOf(const tchar* chars, usize start) const {return start >= data->len ? 0 : _tcspbrk(data->str + start, chars);}
const tchar* String::findLast(const tchar* str) const {return String::findLast(data->str, str);}
const tchar* String::findLastOf(const tchar* chars) const {return String::findLastOf(data->str, chars);}

String& String::replace(const String& needle, const String& replacement)
{
  const tchar* p = data->str;
  const tchar* match = _tcsstr(p, needle);
  if(!match)
    return *this;
  String result(data->len + replacement.data->len * 10);
  for(;;)
  {
    result.append(p, match - p);
    result.append(replacement);
    p = match + needle.data->len;
    match = _tcsstr(p, needle);
    if(!match)
    {
      result.append(p, data->len - (p - data->str));
      return *this = result;
    }
  }
}

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
int String::toInt(const tchar* s) {return _wtoi(s);}
uint String::toUInt(const tchar* s) {return wcstoul(s, 0, 10);}
int64 String::toInt64(const tchar* s) {return _wtoll(s);}
uint64 String::toUInt64(const tchar* s) {return wcstoull(s, 0, 10);}
double String::toDouble(const tchar* s) {return _wtof(s);}
#else
int String::toInt(const tchar* s) {return atoi(s);}
uint String::toUInt(const tchar* s) {return strtoul(s, 0, 10);}
int64 String::toInt64(const tchar* s) {return atoll(s);}
uint64 String::toUInt64(const tchar* s) {return strtoull(s, 0, 10);}
double String::toDouble(const tchar* s) {return atof(s);}
#endif

#ifdef _UNICODE
wchar_t String::toLowerCase(tchar c) {return towlower(c);}
wchar_t String::toUpperCase(tchar c) {return towupper(c);}
bool String::isSpace(tchar c) {return iswspace(c) != 0;}
#endif

#ifdef _UNICODE
bool String::isAlphanumeric(tchar c) { return iswalnum(c) != 0; };
bool String::isAlpha(tchar c) { return iswalpha(c) != 0; };
bool String::isDigit(tchar c) { return iswdigit(c) != 0; };
bool String::isLowerCase(tchar c) { return iswlower(c) != 0; };
bool String::isPrint(tchar c) { return iswprint(c) != 0; };
bool String::isPunct(tchar c) { return iswpunct(c) != 0; };
bool String::isUpperCase(tchar c) { return iswupper(c) != 0; };
bool String::isHexDigit(tchar c) { return iswxdigit(c) != 0; };
#else
bool String::isAlphanumeric(tchar c) { return isalnum((uchar&)c) != 0; };
bool String::isAlpha(tchar c) {return isalpha((uchar&)c) != 0;};
bool String::isDigit(tchar c) { return isdigit((uchar&)c) != 0; };
bool String::isLowerCase(tchar c) { return islower((uchar&)c) != 0; };
bool String::isPrint(tchar c) { return isprint((uchar&)c) != 0; };
bool String::isPunct(tchar c) { return ispunct((uchar&)c) != 0; };
bool String::isUpperCase(tchar c) { return isupper((uchar&)c) != 0; };
bool String::isHexDigit(tchar c) { return isxdigit((uchar&)c) != 0; };
#endif

const tchar* String::find(const tchar* in, const tchar* str) {return _tcsstr(in, str);}
const tchar* String::findOneOf(const tchar* in, const tchar* chars) {return _tcspbrk(in, chars);}
const tchar* String::findLast(const tchar* in, const tchar* str)
{
  const tchar* result = 0;
  const tchar* match = _tcsstr(in, str);
  for(;;)
  {
    if(!match)
      return result;
    result = match;
    match = _tcsstr(match + 1, str);
  }
}
const tchar* String::findLastOf(const tchar* in, const tchar* chars)
{
  const tchar* result = 0;
  const tchar* match = _tcspbrk(in, chars);
  for(;;)
  {
    if(!match)
      return result;
    result = match;
    match = _tcspbrk(match + 1, chars);
  }
}

String String::fromInt(int value)
{
  String result;
  result.printf(_T("%d"), value);
  return result;
}

String String::fromUInt(uint value)
{
  String result;
  result.printf(_T("%u"), value);
  return result;
}

String String::fromInt64(int64 value)
{
  String result;
  result.printf(_T("%lld"), value);
  return result;
}

String String::fromUInt64(uint64 value)
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

String String::token(tchar separator, usize& start) const
{
  const tchar* endStr = find(separator, start);
  if(endStr)
  {
    usize len = endStr - (data->str + start);
    String result = String::substr(start, len);
    start += len + 1;
    return result;
  }
  String result = substr(start);
  start = data->len;
  return result;
}

String String::token(const tchar* separators, usize& start) const
{
  const tchar* p = data->str + start;
  const tchar* endStr = _tcspbrk(p, separators);
  if(endStr)
  {
    usize len = endStr - p;
    String result = substr(start, len);
    start += len + 1;
    return result;
  }
  String result = substr(start);
  start = data->len;
  return result;
}

usize String::split(List<String>& tokens, const tchar* separators, bool skipEmpty) const
{
  tokens.clear();

  const tchar* start = data->str;
  const tchar* p = start, * endStr;
  for(;;)
  {
    endStr = _tcspbrk(p, separators);
    if(endStr)
    {
      usize len = endStr - p;
      if(len)
      {
        tokens.append(substr(p - start, len));
        p = endStr + 1;
      }
      else
      {
        if(!skipEmpty)
          tokens.append(String());
        ++p;
      }
    }
    else
    {
      if(p < start + data->len)
        tokens.append(substr(p - start));
      else if(!skipEmpty)
        tokens.append(String());
      break;
    }
  }
  return tokens.size();
}

usize String::split(HashSet<String>& tokens, const tchar* separators, bool skipEmpty) const
{
  tokens.clear();

  const tchar* start = data->str;
  const tchar* p = start, * endStr;
  for(;;)
  {
    endStr = _tcspbrk(p, separators);
    if(endStr)
    {
      usize len = endStr - p;
      if(len)
      {
        tokens.append(substr(p - start, len));
        p = endStr + 1;
      }
      else
      {
        if(!skipEmpty)
          tokens.append(String());
        ++p;
      }
    }
    else
    {
      if(p < start + data->len)
        tokens.append(substr(p - start));
      else if(!skipEmpty)
        tokens.append(String());
      break;
    }
  }
  return tokens.size();
}

String& String::join(const List<String>& tokens, tchar separator)
{
  clear();
  if(!tokens.isEmpty())
    for(List<String>::Iterator i = tokens.begin(), end = tokens.end();;)
    {
      append(*i);
      if(++i == end)
        break;
      append(separator);
    }
  return *this;
}

String& String::trim(const tchar* chars)
{
  if(data->len)
  {
    const tchar* start = data->str;
    const tchar* p = start;
    const tchar* end = start + data->len;
    for(; p < end; ++p)
      if(!_tcschr(chars, *p))
        break;
    --end;
    for(; end > p; --end)
      if(!_tcschr(chars, *end))
        break;
    ++end;
    size_t newLen = end - p;
    if(newLen != data->len)
      *this = substr(p - start, newLen);
  }
  return *this;
}

String String::fromHex(const byte* data, usize size)
{
  String result;
  result.resize(size * 2);
  tchar* dest = result;
  static const tchar* hex = _T("0123456789ABCDEF");
  for(const byte* src =  data, * end = data + size; src < end; ++src)
  {
    dest[0] = hex[*src >> 4];
    dest[1] = hex[*src & 0xf];
    dest += 2;
  }
  return result;
}
