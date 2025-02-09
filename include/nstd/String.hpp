
#pragma once

#include <nstd/Atomic.hpp>
#include <nstd/Memory.hpp>

template<typename T> class List;
template<typename T> class HashSet;

class String
{
public:
  String() : data(&emptyData) {}

  template<usize N> String(const char(&str)[N]) : data(&_data)
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
      usize capacity = other.data->len | 0x3;
      data = (Data*)new char[(capacity + 1) * sizeof(char) + sizeof(Data)];
      data->str = (char*)((byte*)data + sizeof(Data));
      Memory::copy((char*)data->str, other.data->str, other.data->len * sizeof(char));
      ((char*)data->str)[data->len = other.data->len] = '\0';
      data->ref = 1;
      data->capacity = capacity;
    }
  }

  String(const char* str, usize length)
  {
    usize capacity = length | 0x3;
    data = (Data*)new char[(capacity + 1) * sizeof(char) + sizeof(Data)];
    data->str = (char*)((byte*)data + sizeof(Data));
    Memory::copy((char*)data->str, str, length * sizeof(char));
    ((char*)data->str)[data->len = length] = '\0';
    data->ref = 1;
    data->capacity = capacity;
  }

  String(usize length, char c)
  {
    usize capacity = length | 0x3;
    data = (Data*)new char[(capacity + 1) * sizeof(char) + sizeof(Data)];
    data->str = (char*)((byte*)data + sizeof(Data));
    char* i = (char*)data->str;
    for (char* end = i + length; i < end; ++i)
      *i = c;
    *i =  '\0';
    data->len = length;
    data->ref = 1;
    data->capacity = capacity;
  }

  explicit String(usize capacity)
  {
    data = (Data*)new char[(capacity + 1) * sizeof(char) + sizeof(Data)];
    data->str = (char*)((byte*)data + sizeof(Data));
    *((char*)data->str) = '\0';
    data->len = 0;
    data->ref = 1;
    data->capacity = capacity;
  }

  ~String()
  {
    if(data->ref && Atomic::decrement(data->ref) == 0)
      delete[] (char*)data;
  }

  operator const char*() const
  {
      if(data->str[data->len])
          const_cast<String*>(this)->detach(data->len, data->len);
      return data->str;
  }

  operator const char*()
  {
      if(data->str[data->len])
          const_cast<String*>(this)->detach(data->len, data->len);
      return data->str;
  }

  operator char*()
  {
    detach(data->len, data->len);
    return (char*)data->str;
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
      *(char*)data->str = '\0';
    }
    else
    {
      if(data->ref && Atomic::decrement(data->ref) == 0)
        delete[] (char*)data;

      data = &emptyData;
    }
  }

  void attach(const char* str, usize length)
  {
    if(data->ref && Atomic::decrement(data->ref) == 0)
        delete[] (char*)data;
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
    Memory::copy((char*)data->str, str.data->str, str.data->len * sizeof(char));
    Memory::copy((char*)data->str + str.data->len, copy.data->str, copy.data->len * sizeof(char));
    ((char*)data->str)[data->len = newLen] = '\0';
    return *this;
  }

  String& prepend(const char* str, usize len)
  {
    String copy(*this);
    usize newLen = len + copy.data->len;
    detach(0, newLen);
    Memory::copy((char*)data->str, str, len * sizeof(char));
    Memory::copy((char*)data->str + len, copy.data->str, copy.data->len * sizeof(char));
    ((char*)data->str)[data->len = newLen] = '\0';
    return *this;
  }

  String& append(const String& str)
  {
    usize newLen = data->len + str.data->len;
    detach(data->len, newLen);
    Memory::copy((char*)data->str + data->len, str.data->str, str.data->len * sizeof(char));
    ((char*)data->str)[data->len = newLen] = '\0';
    return *this;
  }

  String& append(const char* str, usize len)
  {
    usize newLen = data->len + len;
    detach(data->len, newLen);
    Memory::copy((char*)data->str + data->len, str, len * sizeof(char));
    ((char*)data->str)[data->len = newLen] = '\0';
    return *this;
  }

  String& append(const char c)
  {
    usize newLen = data->len + 1;
    detach(data->len, newLen);
    ((char*)data->str)[data->len] = c;
    ((char*)data->str)[data->len = newLen] = '\0';
    return *this;
  }

  String& operator=(const String& other)
  {
    Data* otherData = other.data;
    if(otherData->ref)
    {
      Atomic::increment(otherData->ref);
      if(data->ref && Atomic::decrement(data->ref) == 0)
        delete[] (char*)data;
      data = otherData;
    }
    else
    {
      if(data->ref && Atomic::decrement(data->ref) == 0)
        delete[] (char*)data;
      usize capacity = otherData->len | 0x3;
      data = (Data*)new char[(capacity + 1) * sizeof(char) + sizeof(Data)];
      data->str = (char*)((byte*)data + sizeof(Data));
      Memory::copy((char*)data->str, otherData->str, otherData->len * sizeof(char));
      ((char*)data->str)[data->len = otherData->len] = '\0';
      data->ref = 1;
      data->capacity = capacity;
    }
    return *this;
  }

  String& operator+=(const String& other) {return append(other);}
  String& operator+=(char c) { return append(c); }
  String operator+(const String& other) const {return String(*this).append(other);}
  template<usize N> String operator+(const char(&str)[N]) const {return String(*this).append(String(str));}
  bool operator==(const String& other) const {return data->len == other.data->len && Memory::compare(data->str, other.data->str, data->len * sizeof(char)) == 0;}
  template<usize N> bool operator==(const char (&str)[N]) const {return data->len == N - 1 && Memory::compare(data->str, str, (N - 1) * sizeof(char)) == 0;}
  bool operator!=(const String& other) const {return data->len != other.data->len || Memory::compare(data->str, other.data->str, data->len * sizeof(char)) != 0;}
  template<usize N> bool operator!=(const char (&str)[N]) const {return data->len != N - 1 || Memory::compare(data->str, str, (N - 1) * sizeof(char)) != 0;}
  bool operator>(const String& other) const {return compare(other) > 0;}
  bool operator>=(const String& other) const {return compare(other) >= 0;}
  bool operator<(const String& other) const {return compare(other) < 0;}
  bool operator<=(const String& other) const {return compare(other) <= 0;}

  int compare(const String& other) const
  {
    const char* s1 = *this, * s2 = other;
    for(; *s1 == *s2; ++s1, ++s2)
      if(!*s1)
        return 0;
    return (int)*(const uchar*)s1 - *(const uchar*)s2;

  }

  int compare(const String& other, usize len) const
  {
    for(const char* s1 = *this, * s2 = other, * end1 = s1 + len; s1 < end1; ++s1, ++s2)
      if(!*s1 || *s1 != *s2)
        return (int)*(const uchar*)s1 - *(const uchar*)s2;
    return 0;
  }

  int compareIgnoreCase(const String& other) const
  {
    const char* s1 = *this, * s2 = other;
    char c1, c2;
    for(; (c1 = toLowerCase(*s1)) == (c2 = toLowerCase(*s2)); ++s1, ++s2)
      if(!*s1)
        return 0;
    return (int)(const uchar&)c1 - (const uchar&)c2;
  }

  int compareIgnoreCase(const String& other, usize len) const
  {
    char c1, c2;
    for(const char* s1 = *this, * s2 = other, * end1 = s1 + len; s1 < end1; ++s1, ++s2)
      if((c1 = toLowerCase(*s1)) != (c2 = toLowerCase(*s2)) || !*s1)
        return (int)(const uchar&)c1 - (const uchar&)c2;
    return 0;
  }

  bool equalsIgnoreCase(const String& other) const {return data->len == other.data->len && compareIgnoreCase(other) == 0;}
  bool equalsIgnoreCase(const String& other, usize len) const {return compareIgnoreCase(other, len) == 0;}

  const char* find(char c) const
  {
    for(const char* s = data->str, * end = s + data->len; s < end; ++s)
      if(*s == c)
        return s;
    return 0;
  }

  const char* findLast(char c) const
  {
    for(const char * start = data->str, * p = data->str + data->len - 1; p >= start; --p)
      if(*p == c)
        return p;
    return 0;
  }

  const char* find(char c, usize start) const;
  const char* find(const char* str) const;
  const char* find(const char* str, usize start) const;
  const char* findOneOf(const char* chars) const;
  const char* findOneOf(const char* chars, usize start) const;
  const char* findLast(const char* str) const;
  const char* findLastOf(const char* chars) const;

  String& replace(char needle, char replacement)
  {
    detach(data->len, data->len);
    for (char* str = (char*)data->str; *str; ++str)
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

  String& toLowerCase()
  {
    detach(data->len, data->len);
    for (char* str = (char*)data->str; *str; ++str)
      *str = lowerCaseMap[*(uchar*)str];
    return *this;
  }

  String& toUpperCase()
  {
    detach(data->len, data->len);
    for (char* str = (char*)data->str; *str; ++str)
      *str = upperCaseMap[*(uchar*)str];
    return *this;
  }

  int printf(const char* format, ...);
  int scanf(const char* format, ...) const;

  int toInt() const;
  uint toUInt() const;
  int64 toInt64() const;
  uint64 toUInt64() const;
  double toDouble() const;
  bool toBool() const
  {
    if(data->len == 0 || equalsIgnoreCase("false") || *this == "0")
      return false;
    const char* p = *this;
    for(; *p == '0'; ++p);
    if(*p == '.')
    {
      for(++p; *p == '0'; ++p);
      if(!*p && (p[-1] == '0' || *data->str == '0'))
        return false;
    }
    return true;
  }

  String token(char separator, usize& start) const;
  String token(const char* separators, usize& start) const;

  usize split(List<String>& tokens, const char* separators, bool skipEmpty = true) const;
  usize split(HashSet<String>& tokens, const char* separators, bool skipEmpty = true) const;
  String& join(const List<String>& tokens, char separator);

  String& trim(const char* chars = " \t\r\n\v");

public:
  static int toInt(const char* s);
  static uint toUInt(const char* s);
  static int64 toInt64(const char* s);
  static uint64 toUInt64(const char* s);
  static double toDouble(const char* s);

  static char toLowerCase(char c) { return lowerCaseMap[(uchar&)c]; }
  static char toUpperCase(char c) { return upperCaseMap[(uchar&)c]; }
  static bool isSpace(char c) { return (c >= 9 && c <= 13) || c == 32; }

  static bool isAlphanumeric(char c);
  static bool isAlpha(char c);
  static bool isDigit(char c);
  static bool isLowerCase(char c);
  static bool isPrint(char c);
  static bool isPunct(char c);
  static bool isUpperCase(char c);
  static bool isHexDigit(char c);

  static int compare(const char* s1, const char* s2)
  {
    for(; *s1 == *s2; ++s1, ++s2)
      if(!*s1)
        return 0;
    return (int)*(const uchar*)s1 - *(const uchar*)s2;
  }

  static int compare(const char* s1, const char* s2, usize len)
  {
    for(const char* end1 = s1 + len; s1 < end1; ++s1, ++s2)
      if(!*s1 || *s1 != *s2)
        return (int)*(const uchar*)s1 - *(const uchar*)s2;
    return 0;
  }

  static int compareIgnoreCase(const char* s1, const char* s2)
  {
    char c1, c2;
    for(; (c1 = toLowerCase(*s1)) == (c2 = toLowerCase(*s2)); ++s1, ++s2)
      if(!*s1)
        return 0;
    return (int)(const uchar&)c1 - (const uchar&)c2;
  }

  static int compareIgnoreCase(const char* s1, const char* s2, usize len)
  {
    char c1, c2;
    for(const char* end1 = s1 + len; s1 < end1; ++s1, ++s2)
      if((c1 = toLowerCase(*s1)) != (c2 = toLowerCase(*s2)) || !*s1)
        return (int)(const uchar&)c1 - (const uchar&)c2;
    return 0;
  }

  static usize length(const char* s)
  {
    const char* start = s;
    while (*s)
      ++s;
    return (usize)(s - start);
  }

  static const char* find(const char* in, char c)
  {
    for(; *in; ++in)
      if(*in == c)
        return in;
    return 0;
  }

  static const char* findLast(const char* in, char c)
  {
    const char* last = 0;
    for(; *in; ++in)
      if(*in == c)
        last = in;
    return last;
  }

  static const char* find(const char* in, const char* str);
  static const char* findOneOf(const char* in, const char* chars);
  static const char* findLast(const char* in, const char* str);
  static const char* findLastOf(const char* in, const char* chars);

  static String fromInt(int value);
  static String fromUInt(uint value);
  static String fromInt64(int64 value);
  static String fromUInt64(uint64 value);
  static String fromDouble(double value);
  static String fromBool(bool value) {return value ? String("true") : String("false");}
  static String fromCString(const char* str) {return String(str, length(str));}
  static String fromCString(const char* str, usize len) {return String(str, len);}
  static String fromPrintf(const char* format, ...);
  static String fromHex(const byte* data, usize size);
  static String fromBase64(const String& base64);

  static bool startsWith(const char* in, const String& str) {return compare(in, str.data->str, str.data->len) == 0;}

private:
  struct Data
  {
    const char* str;
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
      ((char*)data->str)[data->len = copyLength] = '\0';
      return;
    }

    usize capacity = minCapacity | 0x3;
    Data* newData = (Data*)new char[(capacity + 1) * sizeof(char) + sizeof(Data)];
    newData->str = (char*)((byte*)newData + sizeof(Data));
    if(data->len > 0)
    {
      Memory::copy((char*)newData->str, data->str, (data->len < copyLength ? data->len : copyLength) * sizeof(char));
      ((char*)newData->str)[newData->len = copyLength] = '\0';
    }
    else
    {
      *(char*)newData->str = '\0';
      newData->len = copyLength;
    }
    newData->ref = 1;
    newData->capacity = capacity;

    if(data->ref && Atomic::decrement(data->ref) == 0)
      delete[] (char*)data;

    data = newData;
  }

  static struct EmptyData : public Data
  {
    EmptyData()
    {
      str = (const char*)&len;
      ref = 0;
      len = 0;
    }
  } emptyData;

  static char lowerCaseMap[0x101];
  static char upperCaseMap[0x101];
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
  const char* s = str;
  hashCode *= 16807;
  hashCode ^= s[0];
  hashCode *= 16807;
  hashCode ^= s[len / 2];
  hashCode *= 16807;
  hashCode ^= s[len - (len != 0)];
  return hashCode;
}
