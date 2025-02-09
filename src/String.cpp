
#include <cstdarg>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <cstdlib>

#include <nstd/String.hpp>
#include <nstd/Debug.hpp>
#include <nstd/List.hpp>
#include <nstd/HashSet.hpp>

String::EmptyData String::emptyData;
char String::lowerCaseMap[0x101] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x40\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x5b\x5c\x5d\x5e\x5f\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";
char String::upperCaseMap[0x101] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f\x60\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x7b\x7c\x7d\x7e\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";

int String::printf(const char* format, ...)
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

String String::fromPrintf(const char* format, ...)
{
  String s(200);

  int result;
  va_list ap;
  va_start(ap, format);

  {
    result = vsnprintf((char*)s.data->str, s.data->capacity, format, ap);
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
    result = _vscprintf(format, ap);
#else
    result = vsnprintf(0, 0, format, ap);
#endif
    ASSERT(result >= 0);
    if(result < 0)
      return String();
    s.detach(0, result);
    result = vsnprintf((char*)s.data->str, result + 1, format, ap);
    ASSERT(result >= 0);
    s.data->len = result;
    va_end(ap);
    return s;
  }
}

#if defined(_MSC_VER) && _MSC_VER <= 1600
static int vsscanf(const char* s, const char* fmt, va_list ap)
{
  struct Args
  {
    void *a[32];
  } args;
  for (int i = 0; i < sizeof(args.a) / sizeof(void*); ++i)
    args.a[i] = va_arg(ap, void*);
  return sscanf(s, fmt, args);
}
#define atoll _atoi64
#define strtoull _strtoui64
#endif

int String::scanf(const char* format, ...) const
{
  int result;
  va_list ap;
  va_start(ap, format);
  result = vsscanf(*this, format, ap);
  va_end(ap);
  return result;
}

int String::toInt() const {return atoi(*this);}
uint String::toUInt() const {return strtoul(*this, 0, 10);}
int64 String::toInt64() const {return atoll(*this);}
uint64 String::toUInt64() const {return strtoull(*this, 0, 10);}
double String::toDouble() const {return atof(*this);}

const char* String::find(char c, usize start) const {return start >= data->len ? 0 : strchr(*this + start, c);}
const char* String::find(const char* str) const {return strstr(*this, str);}
const char* String::find(const char* str, usize start) const {return start >= data->len ? 0 : strstr(*this + start, str);}
const char* String::findOneOf(const char* chars) const {return strpbrk(*this, chars);}
const char* String::findOneOf(const char* chars, usize start) const {return start >= data->len ? 0 : strpbrk(*this + start, chars);}
const char* String::findLast(const char* str) const {return String::findLast(*this, str);}
const char* String::findLastOf(const char* chars) const {return String::findLastOf(*this, chars);}

String& String::replace(const String& needle, const String& replacement)
{
  const char* p = data->str;
  const char* match = strstr(p, needle);
  if(!match)
    return *this;
  String result(data->len + replacement.data->len * 10);
  for(;;)
  {
    result.append(p, match - p);
    result.append(replacement);
    p = match + needle.data->len;
    match = strstr(p, needle);
    if(!match)
    {
      result.append(p, data->len - (p - data->str));
      return *this = result;
    }
  }
}

int String::toInt(const char* s) {return atoi(s);}
uint String::toUInt(const char* s) {return strtoul(s, 0, 10);}
int64 String::toInt64(const char* s) {return atoll(s);}
uint64 String::toUInt64(const char* s) {return strtoull(s, 0, 10);}
double String::toDouble(const char* s) {return atof(s);}


bool String::isAlphanumeric(char c) { return isalnum((uchar&)c) != 0; };
bool String::isAlpha(char c) {return isalpha((uchar&)c) != 0;};
bool String::isDigit(char c) { return isdigit((uchar&)c) != 0; };
bool String::isLowerCase(char c) { return islower((uchar&)c) != 0; };
bool String::isPrint(char c) { return isprint((uchar&)c) != 0; };
bool String::isPunct(char c) { return ispunct((uchar&)c) != 0; };
bool String::isUpperCase(char c) { return isupper((uchar&)c) != 0; };
bool String::isHexDigit(char c) { return isxdigit((uchar&)c) != 0; };

const char* String::find(const char* in, const char* str) {return strstr(in, str);}
const char* String::findOneOf(const char* in, const char* chars) {return strpbrk(in, chars);}
const char* String::findLast(const char* in, const char* str)
{
  const char* result = 0;
  const char* match = strstr(in, str);
  for(;;)
  {
    if(!match)
      return result;
    result = match;
    match = strstr(match + 1, str);
  }
}
const char* String::findLastOf(const char* in, const char* chars)
{
  const char* result = 0;
  const char* match = strpbrk(in, chars);
  for(;;)
  {
    if(!match)
      return result;
    result = match;
    match = strpbrk(match + 1, chars);
  }
}

String String::fromInt(int value)
{
  String result;
  result.printf("%d", value);
  return result;
}

String String::fromUInt(uint value)
{
  String result;
  result.printf("%u", value);
  return result;
}

String String::fromInt64(int64 value)
{
  String result;
  result.printf("%lld", value);
  return result;
}

String String::fromUInt64(uint64 value)
{
  String result;
  result.printf("%llu", value);
  return result;
}

String String::fromDouble(double value)
{
  String result;
  result.printf("%f", value);
  return result;
}

String String::token(char separator, usize& start) const
{
  const char* endStr = find(separator, start);
  if(endStr)
  {
    usize len = endStr - (*this + start);
    String result = String::substr(start, len);
    start += len + 1;
    return result;
  }
  String result = substr(start);
  start = data->len;
  return result;
}

String String::token(const char* separators, usize& start) const
{
  const char* p = *this + start;
  const char* endStr = strpbrk(p, separators);
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

usize String::split(List<String>& tokens, const char* separators, bool skipEmpty) const
{
  tokens.clear();

  const char* start = *this;
  const char* p = start, * endStr;
  for(;;)
  {
    endStr = strpbrk(p, separators);
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

usize String::split(HashSet<String>& tokens, const char* separators, bool skipEmpty) const
{
  tokens.clear();

  const char* start = *this;
  const char* p = start, * endStr;
  for(;;)
  {
    endStr = strpbrk(p, separators);
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

String& String::join(const List<String>& tokens, char separator)
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

String& String::trim(const char* chars)
{
  if(data->len)
  {
    const char* start = data->str;
    const char* p = start;
    const char* end = start + data->len;
    for(; p < end; ++p)
      if(!strchr(chars, *p))
        break;
    --end;
    for(; end > p; --end)
      if(!strchr(chars, *end))
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
  char* dest = result;
  const char* hex = "0123456789ABCDEF";
  for(const byte* src =  data, * end = data + size; src < end; ++src)
  {
    dest[0] = hex[*src >> 4];
    dest[1] = hex[*src & 0xf];
    dest += 2;
  }
  return result;
}
