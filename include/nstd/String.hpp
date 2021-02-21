
#pragma once

#include <nstd/Atomic.hpp>
#include <nstd/Memory.hpp>

template<typename T> class List;
template<typename T> class HashSet;

class String
{
public:
  String() : data(&emptyData) {}

  template<usize N> String(const tchar(&str)[N]) : data(&_data)
  {
    _data.ref = 0;
    _data.str = str;
    _data.len = N - 1;
  }

  String(const String& other)
  {
    if(other.data->ref)
    {
      data = other.data;
      Atomic::increment(data->ref);
    }
    else if(other.data == &emptyData)
      data = &emptyData;
    else
    {
      usize capacity;
      data = (Data*)Memory::alloc((other.data->len + 1) * sizeof(tchar) + sizeof(Data), capacity);
      data->str = (tchar*)((byte*)data + sizeof(Data));
      Memory::copy((tchar*)data->str, other.data->str, other.data->len * sizeof(tchar));
      ((tchar*)data->str)[data->len = other.data->len] = _T('\0');
      data->ref = 1;
      data->capacity = (capacity - sizeof(Data)) / sizeof(tchar) - 1;
    }
  }

  String(const tchar* str, usize length)
  {
    usize capacity;
    data = (Data*)Memory::alloc((length + 1) * sizeof(tchar) + sizeof(Data), capacity);
    data->str = (tchar*)((byte*)data + sizeof(Data));
    Memory::copy((tchar*)data->str, str, length * sizeof(tchar));
    ((tchar*)data->str)[data->len = length] = _T('\0');
    data->ref = 1;
    data->capacity = (capacity - sizeof(Data)) / sizeof(tchar) - 1;
  }

  String(usize length, tchar c)
  {
    usize capacity;
    data = (Data*)Memory::alloc((length + 1) * sizeof(tchar) + sizeof(Data), capacity);
    data->str = (tchar*)((byte*)data + sizeof(Data));
    tchar* i = (tchar*)data->str;
    for (tchar* end = i + length; i < end; ++i)
      *i = c;
    *i =  _T('\0');
    data->len = length;
    data->ref = 1;
    data->capacity = (capacity - sizeof(Data)) / sizeof(tchar) - 1;
  }

  explicit String(usize capacity)
  {
    data = (Data*)Memory::alloc((capacity + 1) * sizeof(tchar) + sizeof(Data), capacity);
    data->str = (tchar*)((byte*)data + sizeof(Data));
    *((tchar*)data->str) = _T('\0');
    data->len = 0;
    data->ref = 1;
    data->capacity = (capacity - sizeof(Data)) / sizeof(tchar) - 1;
  }

  ~String()
  {
    if(data->ref && Atomic::decrement(data->ref) == 0)
      Memory::free(data);
  }

  operator const tchar*() const
  {
      if(data->str[data->len])
          const_cast<String*>(this)->detach(data->len, data->len);
      return data->str;
  }

  operator const tchar*()
  {
      if(data->str[data->len])
          const_cast<String*>(this)->detach(data->len, data->len);
      return data->str;
  }

  operator tchar*()
  {
    detach(data->len, data->len);
    return (tchar*)data->str;
  }
  
  usize length() const {return data->len;}

  usize capacity() const
  {
    if(data->ref == 1)
      return data->capacity;
    return 0;
  }

  void clear()
  {
    if(data->ref == 1)
    {
      data->len = 0;
      *(tchar*)data->str = _T('\0');
    }
    else
    {
      if(data->ref && Atomic::decrement(data->ref) == 0)
        Memory::free(data);

      data = &emptyData;
    }
  }

  void attach(const tchar* str, usize length)
  {
    if(data->ref && Atomic::decrement(data->ref) == 0)
        Memory::free(data);
    data = &_data;
    _data.ref = 0;
    _data.str = str;
    _data.len = length;
  }

  void detach() {detach(data->len, data->len);}
  bool isEmpty() const {return data->len == 0;}
  void resize(usize length) {detach(length, length);}
  void reserve(usize size) {detach(data->len, size < data->len ? data->len : size);} // todo: optimize this method, use it in append methods

  String& prepend(const String& str)
  {
    String copy(*this);
    usize newLen = str.data->len + copy.data->len;
    detach(0, newLen);
    Memory::copy((tchar*)data->str, str.data->str, str.data->len * sizeof(tchar));
    Memory::copy((tchar*)data->str + str.data->len, copy.data->str, copy.data->len * sizeof(tchar));
    ((tchar*)data->str)[data->len = newLen] = _T('\0');
    return *this;
  }

  String& prepend(const tchar* str, usize len)
  {
    String copy(*this);
    usize newLen = len + copy.data->len;
    detach(0, newLen);
    Memory::copy((tchar*)data->str, str, len * sizeof(tchar));
    Memory::copy((tchar*)data->str + len, copy.data->str, copy.data->len * sizeof(tchar));
    ((tchar*)data->str)[data->len = newLen] = _T('\0');
    return *this;
  }

  String& append(const String& str)
  {
    usize newLen = data->len + str.data->len;
    detach(data->len, newLen);
    Memory::copy((tchar*)data->str + data->len, str.data->str, str.data->len * sizeof(tchar));
    ((tchar*)data->str)[data->len = newLen] = _T('\0');
    return *this;
  }

  String& append(const tchar* str, usize len)
  {
    usize newLen = data->len + len;
    detach(data->len, newLen);
    Memory::copy((tchar*)data->str + data->len, str, len * sizeof(tchar));
    ((tchar*)data->str)[data->len = newLen] = _T('\0');
    return *this;
  }

  String& append(const tchar c)
  {
    usize newLen = data->len + 1;
    detach(data->len, newLen);
    ((tchar*)data->str)[data->len] = c;
    ((tchar*)data->str)[data->len = newLen] = _T('\0');
    return *this;
  }

  String& operator=(const String& other)
  {
    Data* otherData = other.data;
    if(otherData->ref)
    {
      Atomic::increment(otherData->ref);
      if(data->ref && Atomic::decrement(data->ref) == 0)
        Memory::free(data);
      data = otherData;
    }
    else
    {
      if(data->ref && Atomic::decrement(data->ref) == 0)
        Memory::free(data);
      usize capacity;
      data = (Data*)Memory::alloc((otherData->len + 1) * sizeof(tchar) + sizeof(Data), capacity);
      data->str = (tchar*)((byte*)data + sizeof(Data));
      Memory::copy((tchar*)data->str, otherData->str, otherData->len * sizeof(tchar));
      ((tchar*)data->str)[data->len = otherData->len] = _T('\0');
      data->ref = 1;
      data->capacity = (capacity - sizeof(Data)) / sizeof(tchar) - 1;
    }
    return *this;
  }

  String& operator+=(const String& other) {return append(other);}
  String& operator+=(tchar c) { return append(c); }
  String operator+(const String& other) const {return String(*this).append(other);}
  template<usize N> String operator+(const tchar(&str)[N]) const {return String(*this).append(String(str));}
  bool operator==(const String& other) const {return data->len == other.data->len && Memory::compare(data->str, other.data->str, data->len * sizeof(tchar)) == 0;}
  template<usize N> bool operator==(const tchar (&str)[N]) const {return data->len == N - 1 && Memory::compare(data->str, str, (N - 1) * sizeof(tchar)) == 0;}
  bool operator!=(const String& other) const {return data->len != other.data->len || Memory::compare(data->str, other.data->str, data->len * sizeof(tchar)) != 0;}
  template<usize N> bool operator!=(const tchar (&str)[N]) const {return data->len != N - 1 || Memory::compare(data->str, str, (N - 1) * sizeof(tchar)) != 0;}
  bool operator>(const String& other) const {return compare(other) > 0;}
  bool operator>=(const String& other) const {return compare(other) >= 0;}
  bool operator<(const String& other) const {return compare(other) < 0;}
  bool operator<=(const String& other) const {return compare(other) <= 0;}

  int compare(const String& other) const
  {
    const tchar* s1 = data->str, * s2 = other.data->str;
    for(; *s1 == *s2; ++s1, ++s2)
      if(!*s1)
        return 0;
    return (int)*(const utchar*)s1 - *(const utchar*)s2;

  }

  int compare(const String& other, usize len) const
  {
    for(const tchar* s1 = data->str, * s2 = other.data->str, * end1 = s1 + len; s1 < end1; ++s1, ++s2)
      if(!*s1 || *s1 != *s2)
        return (int)*(const utchar*)s1 - *(const utchar*)s2;
    return 0;
  }

  int compareIgnoreCase(const String& other) const
  {
    const tchar* s1 = data->str, * s2 = other.data->str;
    tchar c1, c2;
    for(; (c1 = toLowerCase(*s1)) == (c2 = toLowerCase(*s2)); ++s1, ++s2)
      if(!*s1)
        return 0;
    return (int)(const utchar&)c1 - (const utchar&)c2;
  }

  int compareIgnoreCase(const String& other, usize len) const
  {
    tchar c1, c2;
    for(const tchar* s1 = data->str, * s2 = other.data->str, * end1 = s1 + len; s1 < end1; ++s1, ++s2)
      if((c1 = toLowerCase(*s1)) != (c2 = toLowerCase(*s2)) || !*s1)
        return (int)(const utchar&)c1 - (const utchar&)c2;
    return 0;
  }

  bool equalsIgnoreCase(const String& other) const {return data->len == other.data->len && compareIgnoreCase(other) == 0;}
  bool equalsIgnoreCase(const String& other, usize len) const {return compareIgnoreCase(other, len) == 0;}

  const tchar* find(tchar c) const
  {
    for(const tchar* s = data->str; *s; ++s)
      if(*s == c)
        return s;
    return 0;
  }

  const tchar* findLast(tchar c) const
  {
    for(const tchar * start = data->str, * p = data->str + data->len - 1; p >= start; --p)
      if(*p == c)
        return p;
    return 0;
  }

  const tchar* find(tchar c, usize start) const;
  const tchar* find(const tchar* str) const;
  const tchar* find(const tchar* str, usize start) const;
  const tchar* findOneOf(const tchar* chars) const;
  const tchar* findOneOf(const tchar* chars, usize start) const;
  const tchar* findLast(const tchar* str) const;
  const tchar* findLastOf(const tchar* chars) const;

  String& replace(tchar needle, tchar replacement)
  {
    detach(data->len, data->len);
    for (tchar* str = (tchar*)data->str; *str; ++str)
      if(*str == needle)
        *str = replacement;
    return *this;
  }

  String& replace(const String& needle, const String& replacement);

  bool startsWith(const String& str) const {return data->len >= str.data->len && Memory::compare(data->str, str.data->str, str.data->len) == 0;}
  bool endsWith(const String& str) const {return data->len >= str.data->len && Memory::compare(data->str + data->len - str.data->len, str.data->str, str.data->len) == 0;}

  String substr(ssize start, ssize length = -1) const
  {
    if(start < 0)
    {
      start = (ssize)data->len + start;
      if(start < 0)
        start = 0;
    }
    else if((usize)start > data->len)
      start = data->len;

    usize end;
    if(length >= 0)
    {
      end = (usize)start + (usize)length;
      if(end > data->len)
        end = data->len;
    }
    else
      end = data->len;

    length = end - start;
    return String(data->str + start, length);
  }

#ifdef _UNICODE
  String& toLowerCase();
#else
  String& toLowerCase()
  {
    detach(data->len, data->len);
    for (char* str = (char*)data->str; *str; ++str)
      *str = lowerCaseMap[*(uchar*)str];
    return *this;
  }
#endif

#ifdef _UNICODE
  String& toUpperCase();
#else
  String& toUpperCase()
  {
    detach(data->len, data->len);
    for (char* str = (char*)data->str; *str; ++str)
      *str = upperCaseMap[*(uchar*)str];
    return *this;
  }
#endif

  int printf(const tchar* format, ...);
  int scanf(const tchar* format, ...) const;

  int toInt() const;
  uint toUInt() const;
  int64 toInt64() const;
  uint64 toUInt64() const;
  double toDouble() const;
  bool toBool() const
  {
    if(data->len == 0 || equalsIgnoreCase(_T("false")) || *this == _T("0"))
      return false;
    const tchar* p = data->str;
    for(; *p == _T('0'); ++p);
    if(*p == _T('.'))
    {
      for(++p; *p == _T('0'); ++p);
      if(!*p && (p[-1] == _T('0') || *data->str == _T('0')))
        return false;
    }
    return true;
  }

  String token(tchar separator, usize& start) const;
  String token(const tchar* separators, usize& start) const;

  usize split(List<String>& tokens, const tchar* separators, bool skipEmpty = true) const;
  usize split(HashSet<String>& tokens, const tchar* separators, bool skipEmpty = true) const;
  String& join(const List<String>& tokens, tchar separator);

  String& trim(const tchar* chars = _T(" \t\r\n\v"));

public:
  static int toInt(const tchar* s);
  static uint toUInt(const tchar* s);
  static int64 toInt64(const tchar* s);
  static uint64 toUInt64(const tchar* s);
  static double toDouble(const tchar* s);

#ifdef _UNICODE
  static tchar toLowerCase(tchar c);
  static tchar toUpperCase(tchar c);
  static bool isSpace(tchar c);
#else
  static char toLowerCase(tchar c) { return lowerCaseMap[(uchar&)c]; }
  static char toUpperCase(tchar c) { return upperCaseMap[(uchar&)c]; }
  static bool isSpace(tchar c) { return (c >= 9 && c <= 13) || c == 32; }
#endif

  static bool isAlphanumeric(tchar c);
  static bool isAlpha(tchar c);
  static bool isDigit(tchar c);
  static bool isLowerCase(tchar c);
  static bool isPrint(tchar c);
  static bool isPunct(tchar c);
  static bool isUpperCase(tchar c);
  static bool isHexDigit(tchar c);

  static int compare(const tchar* s1, const tchar* s2)
  {
    for(; *s1 == *s2; ++s1, ++s2)
      if(!*s1)
        return 0;
    return (int)*(const utchar*)s1 - *(const utchar*)s2;
  }

  static int compare(const tchar* s1, const tchar* s2, usize len)
  {
    for(const tchar* end1 = s1 + len; s1 < end1; ++s1, ++s2)
      if(!*s1 || *s1 != *s2)
        return (int)*(const utchar*)s1 - *(const utchar*)s2;
    return 0;
  }

  static int compareIgnoreCase(const tchar* s1, const tchar* s2)
  {
    tchar c1, c2;
    for(; (c1 = toLowerCase(*s1)) == (c2 = toLowerCase(*s2)); ++s1, ++s2)
      if(!*s1)
        return 0;
    return (int)(const utchar&)c1 - (const utchar&)c2;
  }

  static int compareIgnoreCase(const tchar* s1, const tchar* s2, usize len)
  {
    tchar c1, c2;
    for(const tchar* end1 = s1 + len; s1 < end1; ++s1, ++s2)
      if((c1 = toLowerCase(*s1)) != (c2 = toLowerCase(*s2)) || !*s1)
        return (int)(const utchar&)c1 - (const utchar&)c2;
    return 0;
  }

  static usize length(const tchar* s)
  {
    const tchar* start = s;
    while (*s)
      ++s;
    return (usize)(s - start);
  }

  static const tchar* find(const tchar* in, tchar c)
  {
    for(; *in; ++in)
      if(*in == c)
        return in;
    return 0;
  }

  static const tchar* findLast(const tchar* in, tchar c)
  {
    const tchar* last = 0;
    for(; *in; ++in)
      if(*in == c)
        last = in;
    return last;
  }

  static const tchar* find(const tchar* in, const tchar* str);
  static const tchar* findOneOf(const tchar* in, const tchar* chars);
  static const tchar* findLast(const tchar* in, const tchar* str);
  static const tchar* findLastOf(const tchar* in, const tchar* chars);

  static String fromInt(int value);
  static String fromUInt(uint value);
  static String fromInt64(int64 value);
  static String fromUInt64(uint64 value);
  static String fromDouble(double value);
  static String fromBool(bool value) {return value ? String(_T("true")) : String(_T("false"));}
  static String fromCString(const tchar* str) {return String(str, length(str));}
  static String fromCString(const tchar* str, usize len) {return String(str, len);}
  static String fromPrintf(const tchar* format, ...);
  static String fromHex(const byte* data, usize size);

  static bool startsWith(const tchar* in, const String& str) {return compare(in, str.data->str, str.data->len) == 0;}

private:
  struct Data
  {
    const tchar* str;
    usize len;
    usize capacity;
    volatile usize ref;
  };

  Data* data;
  Data _data;

  void detach(usize copyLength, usize minCapacity)
  {
#ifdef ASSERT
    ASSERT(copyLength <= minCapacity);
#endif
    if(data->ref == 1 && minCapacity <= data->capacity)
    {
      ((tchar*)data->str)[data->len = copyLength] = _T('\0');
      return;
    }

    usize capacity;
    Data* newData = (Data*)Memory::alloc((minCapacity + 1) * sizeof(tchar) + sizeof(Data), capacity);
    newData->str = (tchar*)((byte*)newData + sizeof(Data));
    if(data->len > 0)
    {
      Memory::copy((tchar*)newData->str, data->str, (data->len < copyLength ? data->len : copyLength) * sizeof(tchar));
      ((tchar*)newData->str)[newData->len = copyLength] = _T('\0');
    }
    else
    {
      *(tchar*)newData->str = _T('\0');
      newData->len = copyLength;
    }
    newData->ref = 1;
    newData->capacity = (capacity - sizeof(Data)) / sizeof(tchar) - 1;

    if(data->ref && Atomic::decrement(data->ref) == 0)
      Memory::free(data);

    data = newData;
  }

  static struct EmptyData : public Data
  {
    EmptyData()
    {
      str = (const tchar*)&len;
      ref = 0;
      len = 0;
    }
  } emptyData;

#ifndef _UNICODE
  static tchar lowerCaseMap[0x101];
  static tchar upperCaseMap[0x101];
#endif
};

/**
* Compute a hash code of a string.
* @param str The string.
* @return The hash code
*/
inline usize hash(const String& str)
{
  usize len;
  usize hashCode = (len = str.length());
  const tchar* s = str;
  hashCode *= 16807;
  hashCode ^= s[0];
  hashCode *= 16807;
  hashCode ^= s[len / 2];
  hashCode *= 16807;
  hashCode ^= s[len - (len != 0)];
  return hashCode;
}
