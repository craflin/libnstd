
#pragma once

#include <nstd/Atomic.h>
#include <nstd/Memory.h>

class String
{
public:
  String() : data(&emptyData) {}

  template<size_t N> String(const tchar_t(&str)[N]) : data(&_data)
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
    else
    {
      data = (Data*)Memory::alloc((other.data->len + 1) * sizeof(tchar_t) + sizeof(Data));
      data->str = (tchar_t*)((byte_t*)data + sizeof(Data));
      Memory::copy((tchar_t*)data->str, other.data->str, (other.data->len + 1) * sizeof(tchar_t));
      data->len = other.data->len;
      data->ref = 1;
    }
  }

  String(const tchar_t* str, size_t length)
  {
    data = (Data*)Memory::alloc((length + 1) * sizeof(tchar_t) + sizeof(Data));
    data->str = (tchar_t*)((byte_t*)data + sizeof(Data));
    Memory::copy((tchar_t*)data->str, str, length * sizeof(tchar_t));
    data->len = length;
    ((tchar_t*)data->str)[length] = _T('\0');
    data->ref = 1;
  }

  explicit String(const tchar_t* str)
  {
    size_t length = String::length(str);
    data = (Data*)Memory::alloc((length + 1) * sizeof(tchar_t) + sizeof(Data));
    data->str = (tchar_t*)((byte_t*)data + sizeof(Data));
    Memory::copy((tchar_t*)data->str, str, length * sizeof(tchar_t));
    data->len = length;
    ((tchar_t*)data->str)[length] = _T('\0');
    data->ref = 1;
  }

  explicit String(size_t capacity)
  {
    data = (Data*)Memory::alloc((capacity + 1) * sizeof(tchar_t) + sizeof(Data));
    data->str = (tchar_t*)((byte_t*)data + sizeof(Data));
    *((tchar_t*)data->str) = _T('\0');
    data->len = 0;
    data->ref = 1;
  }

  ~String()
  {
    if(data->ref && Atomic::decrement(data->ref) == 0)
      Memory::free(data);
  }

  operator const tchar_t*() const {return data->str;}

  operator tchar_t*()
  {
    detach(data->len, data->len);
    return (tchar_t*)data->str;
  }
  
  size_t length() const {return data->len;}

  size_t capacity() const
  {
    if(data->ref == 1)
      return (Memory::size(data) - sizeof(Data)) / sizeof(tchar_t) - 1;
    return 0;
  }

  void_t clear()
  {
    if(data->ref == 1)
    {
      data->len = 0;
      *(tchar_t*)data->str = _T('\0');
    }
    else
    {
      if(data->ref && Atomic::decrement(data->ref) == 0)
        Memory::free(data);

      data = &emptyData;
    }
  }

  bool_t isEmpty() const {return data->len == 0;}

  void_t resize(size_t length)
  {
    detach(length, length);
  }

  void_t reserve(size_t size)
  {
    detach(data->len, size < data->len ? data->len : size);
  }

  String& prepend(const String& str)
  {
    String copy(*this);
    size_t newLen = str.data->len + copy.data->len;
    detach(0, newLen);
    Memory::copy((tchar_t*)data->str, str.data->str, str.data->len * sizeof(tchar_t));
    Memory::copy((tchar_t*)data->str + str.data->len, copy.data->str, (copy.data->len + 1) * sizeof(tchar_t));
    data->len = newLen;
    return *this;
  }

  String& append(const String& str)
  {
    size_t newLen = data->len + str.data->len;
    detach(data->len, newLen);
    Memory::copy((tchar_t*)data->str + data->len, str.data->str, (str.data->len + 1) * sizeof(tchar_t));
    data->len = newLen;
    return *this;
  }

  String& append(const tchar_t* str, size_t len)
  {
    size_t newLen = data->len + len;
    detach(data->len, newLen);
    Memory::copy((tchar_t*)data->str + data->len, str, len * sizeof(tchar_t));
    ((tchar_t*)data->str)[newLen] = _T('\0');
    data->len = newLen;
    return *this;
  }

  String& append(const tchar_t c)
  {
    size_t newLen = data->len + 1;
    detach(data->len, newLen);
    ((tchar_t*)data->str)[data->len] = c;
    ((tchar_t*)data->str)[newLen] = _T('\0');
    data->len = newLen;
    return *this;
  }

  String& operator=(const String& other)
  {
    if(data->ref && Atomic::decrement(data->ref) == 0)
      Memory::free(data);

    if(other.data->ref)
    {
      data = other.data;
      Atomic::increment(data->ref);
    }
    else
    {
      data = (Data*)Memory::alloc((other.data->len + 1) * sizeof(tchar_t)+ sizeof(Data));
      data->str = (tchar_t*)((byte_t*)data + sizeof(Data));
      Memory::copy((tchar_t*)data->str, other.data->str, (other.data->len + 1) * sizeof(tchar_t));
      data->len = other.data->len;
      data->ref = 1;
    }
    return *this;
  }

  String& operator+=(const String& other) {return append(other);}

  String& operator+=(tchar_t c) { return append(c); }

  String operator+(const String& other) const {return String(*this).append(other);}

  bool operator==(const String& other) const {return data->len == other.data->len && Memory::compare(data->str, other.data->str, data->len * sizeof(tchar_t)) == 0;}

  template<size_t N> bool operator==(const tchar_t (&str)[N]) const {return data->len == N - 1 && Memory::compare(data->str, str, (N - 1) * sizeof(tchar_t)) == 0;}

  bool operator!=(const String& other) const {return data->len != other.data->len || Memory::compare(data->str, other.data->str, data->len * sizeof(tchar_t)) != 0;}

  template<size_t N> bool operator!=(const tchar_t (&str)[N]) const {return data->len != N - 1 || Memory::compare(data->str, str, (N - 1) * sizeof(tchar_t)) != 0;}

  bool_t operator>(const String& other) const {return compare(other) > 0;}

  bool_t operator>=(const String& other) const {return compare(other) >= 0;}

  bool_t operator<(const String& other) const {return compare(other) < 0;}

  bool_t operator<=(const String& other) const {return compare(other) <= 0;}

  int_t compare(const String& other) const
  {
    const tchar_t* s1 = data->str, * s2 = other.data->str;
    while(*s1 && *s1 == *s2)
        ++s1,++s2;
    return (int_t)*(const tchar_t*)s1 - *(const tchar_t*)s2;
  }

  const tchar_t* find(tchar_t c) const
  {
    for(const tchar_t* s = data->str; *s; ++s)
      if(*s == c)
        return s;
    return 0;
  }

  const tchar_t* findLast(tchar_t c) const
  {
    for(const tchar_t * start = data->str, * p = data->str + data->len - 1; p >= start; --p)
      if(*p == c)
        return p;
    return 0;
  }

  const tchar_t* find(const tchar_t* str) const;
  const tchar_t* findOneOf(const tchar_t* chars) const;
  const tchar_t* findLast(const tchar_t* str) const;
  const tchar_t* findLastOf(const tchar_t* chars) const;

  String substr(ssize_t start, ssize_t length = -1) const
  {
    if(start < 0)
    {
      start = (ssize_t)data->len + start;
      if(start < 0)
        start = 0;
    }
    else if((size_t)start > data->len)
      start = data->len;

    size_t end;
    if(length >= 0)
    {
      end = (size_t)start + (size_t)length;
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
    for (char_t* str = (char_t*)data->str; *str; ++str)
      *str = lowerCaseMap[*(uchar_t*)str];
    return *this;
  }
#endif

#ifdef _UNICODE
  String& toUpperCase();
#else
  String& toUpperCase()
  {
    detach(data->len, data->len);
    for (char_t* str = (char_t*)data->str; *str; ++str)
      *str = upperCaseMap[*(uchar_t*)str];
    return *this;
  }
#endif

  int_t printf(const tchar_t* format, ...);

#ifdef _UNICODE
  static tchar_t toLowerCase(tchar_t c);
  static tchar_t toUpperCase(tchar_t c);
  static bool_t isSpace(tchar_t c);
#else
  static char_t toLowerCase(char_t c) { return lowerCaseMap[(uchar_t&)c]; }
  static char_t toUpperCase(char_t c) { return upperCaseMap[(uchar_t&)c]; }
  static bool_t isSpace(char_t c) { return (c >= 9 && c <= 13) || c == 32; }
#endif

  static bool_t isAlnum(tchar_t c);
  static bool_t isAlpha(tchar_t c);
  static bool_t isDigit(tchar_t c);
  static bool_t isLower(tchar_t c);
  static bool_t isPrint(tchar_t c);
  static bool_t isPunct(tchar_t c);
  static bool_t isUpper(tchar_t c);
  static bool_t isXDigit(tchar_t c);

  static int_t compare(const tchar_t* s1, const tchar_t* s2)
  {
    while(*s1 && *s1 == *s2)
        ++s1,++s2;
    return (int_t)*(const tchar_t*)s1 - *(const tchar_t*)s2;
  }

  static size_t length(const tchar_t* s)
  {
    const tchar_t* start = s;
    while (*s)
      ++s;
    return (size_t)(s - start);
  }

  /**
  * Compute a hash code for this string.
  * @return The hash code
  */
  explicit operator size_t() const
  {
    size_t len;
    size_t hashCode = (len = data->len);
    const tchar_t* str = data->str;
    hashCode *= 16807;
    hashCode ^= str[0];
    hashCode *= 16807;
    hashCode ^= str[len / 2];
    hashCode *= 16807;
    hashCode ^= str[len - (len != 0)];
    return hashCode;
  }

private:
  struct Data
  {
    const tchar_t* str;
    size_t len;
    volatile size_t ref;
  };

  Data* data;
  Data _data;

  void_t detach(size_t copyLength, size_t minCapacity);

  static struct EmptyData : public Data
  {
    EmptyData()
    {
      str = (const tchar_t*)&len;
      ref = 0;
      len = 0;
    }
  } emptyData;

#ifndef _UNICODE
  static tchar_t lowerCaseMap[0x100];
  static tchar_t upperCaseMap[0x100];
#endif
};
