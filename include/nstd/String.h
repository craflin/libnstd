
#pragma once

#include <nstd/Atomic.h>
#include <nstd/Memory.h>

class String
{
public:
  String() : data(&emptyData) {}

  template<size_t N> String(const char_t (&str)[N]) : data(&_data)
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
      data = (Data*)Memory::alloc(other.data->len + (1 + sizeof(Data)));
      data->str = (char_t*)data + sizeof(Data);
      Memory::copy((char_t*)data->str, other.data->str, other.data->len + 1);
      data->len = other.data->len;
      data->ref = 1;
    }
  }

  String(const char_t* str, uint_t length)
  {
    data = (Data*)Memory::alloc(length + (1 + sizeof(Data)));
    data->str = (char_t*)data + sizeof(Data);
    Memory::copy((char_t*)data->str, str, length);
    data->len = length;
    ((char_t*)data->str)[length] = '\0';
    data->ref = 1;
  }

  explicit String(uint_t capacity)
  {
    data = (Data*)Memory::alloc(capacity + (1 + sizeof(Data)));
    data->str = (char_t*)data + sizeof(Data);
    *((char_t*)data->str) = '\0';
    data->len = 0;
    data->ref = 1;
  }

  ~String()
  {
    if(data->ref && Atomic::decrement(data->ref) == 0)
      Memory::free(data);
  }

  operator const char_t*() const {return data->str;}
  
  uint_t length() const {return data->len;}

  uint_t capacity() const
  {
    if(data->ref == 1)
      return (uint_t)(Memory::size(data) - (1 + sizeof(Data)));
    return 0;
  }

  void_t clear()
  {
    if(data->ref == 1)
    {
      data->len = 0;
      *(char_t*)data->str = '\0';
    }
    else
    {
      if(data->ref && Atomic::decrement(data->ref) == 0)
        Memory::free(data);

      data = &emptyData;
    }
  }

  bool_t empty() const {return data->len == 0;}

  void_t String::resize(uint_t length)
  {
    detach(length, length);
  }

  void_t String::reserve(uint_t size)
  {
    detach(data->len, size < data->len ? data->len : size);
  }

  String& String::prepend(const String& str)
  {
    String copy(*this);
    uint_t newLen = str.data->len + copy.data->len;
    detach(0, newLen);
    Memory::copy((char_t*)data->str, str.data->str, str.data->len);
    Memory::copy((char_t*)data->str + str.data->len, copy.data->str, copy.data->len);
    return *this;
  }

  String& String::append(const String& str)
  {
    uint_t newLen = data->len + str.data->len;
    detach(data->len, newLen);
    Memory::copy((char_t*)data->str + data->len, str.data->str, str.data->len);
    data->len = newLen;
    return *this;
  }

  String& String::append(const char_t c)
  {
    uint_t newLen = data->len + 1;
    detach(data->len, newLen);
    ((char_t*)data->str)[data->len] = c;
    ((char_t*)data->str)[newLen] = '\0';
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
      data = (Data*)Memory::alloc(other.data->len + (1 + sizeof(Data)));
      data->str = (char_t*)data + sizeof(Data);
      Memory::copy((char_t*)data->str, other.data->str, other.data->len + 1);
      data->len = other.data->len;
      data->ref = 1;
    }
    return *this;
  }

  String& operator+=(const String& other) {return append(other);}

  String operator+(const String& other) const {return String(*this).append(other);}

  bool operator==(const String& other) const {return data->len == other.data->len && Memory::compare(data->str, other.data->str, data->len) == 0;}

  template<size_t N> bool operator==(const char_t (&str)[N]) const {return data->len == N - 1 && Memory::compare(data->str, str, N - 1) == 0;}

  bool operator!=(const String& other) const {return data->len != other.data->len || Memory::compare(data->str, other.data->str, data->len) != 0;}

  template<size_t N> bool operator!=(const char_t (&str)[N]) const {return data->len != N - 1 || Memory::compare(data->str, str, N - 1) != 0;}

  bool_t operator>(const String& other) const {return cmp(other) > 0;}

  bool_t operator>=(const String& other) const {return cmp(other) >= 0;}

  bool_t operator<(const String& other) const {return cmp(other) < 0;}

  bool_t operator<=(const String& other) const {return cmp(other) <= 0;}

  int_t cmp(const String& other) const
  {
    const char_t* s1 = data->str, * s2 = other.data->str;
    while(*s1 && *s1 == *s2)
        ++s1,++s2;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
  }

  String substr(int_t start, int_t length = -1) const
  {
    if(start < 0)
    {
      start = (int_t)data->len + start;
      if(start < 0)
        start = 0;
    }
    else if((uint_t)start > data->len)
      start = data->len;

    int_t end;
    if(length >= 0)
    {
      end = start + length;
      if((uint_t)end > data->len)
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
    for(char_t* str = (char_t*)data->str; *str; ++str)
      *str = lowerCaseMap[*(uchar_t*)str];
    return *this;
  }

  String& toUpperCase()
  {
    detach(data->len, data->len);
    for(char_t* str = (char_t*)data->str; *str; ++str)
      *str = upperCaseMap[*(uchar_t*)str];
    return *this;
  }

  int_t printf(const char_t* format, ...);

  static char_t toLowerCase(char_t c) {return lowerCaseMap[(uchar_t&)c];}
  static char_t toUpperCase(char_t c) {return upperCaseMap[(uchar_t&)c];}
  static bool_t isSpace(char_t c) {return (c >= 9 && c <= 13) || c == 32;}

  static bool_t isAlnum(char_t c);
  static bool_t isAlpha(char_t c);
  static bool_t isDigit(char_t c);
  static bool_t isLower(char_t c);
  static bool_t isPrint(char_t c);
  static bool_t isPunct(char_t c);
  static bool_t isUpper(char_t c);
  static bool_t isXDigit(char_t c);

  /**
  * Compute a hash code for this string.
  * @return The hash code
  */
  operator size_t() const
  {
    uint_t len;
    size_t hashCode = (len = data->len);
    const char_t* str = data->str;
    hashCode *= 16807;
    hashCode ^= str[0];
    hashCode *= 16807;
    hashCode ^= str[len >> 1];
    hashCode *= 16807;
    hashCode ^= str[len - (len != 0)];
    return hashCode;
  }

private:
  struct Data
  {
    const char_t* str;
    uint_t len;
    size_t ref;
  };

  Data* data;
  Data _data;

  void_t String::detach(uint_t copyLength, uint_t minCapacity);

  static struct EmptyData : public Data
  {
    EmptyData()
    {
      str = (const char_t*)&len;
      ref = 0;
      len = 0;
    }
  } emptyData;

  static char_t lowerCaseMap[0x100];
  static char_t upperCaseMap[0x100];
};

