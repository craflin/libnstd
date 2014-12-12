
#pragma once

#include <nstd/String.h>

class Unicode
{
public:
  static String toString(uint32_t ch)
  {
    String result;
    append(ch, result);
    return result;
  }

  static bool_t append(uint32_t ch, String& str)
  {
#ifdef _UNICODE
    // todo
#else
    if(ch < 0x80)
    {
      str.append((char_t)ch);
      return true;
    }
    if(ch < 0x800)
    {
      str.append((ch>>6) | 0xC0);
      str.append((ch & 0x3F) | 0x80);
      return true;
    }
    if(ch < 0x10000)
    {
      str.append((ch>>12) | 0xE0);
      str.append(((ch>>6) & 0x3F) | 0x80);
      str.append((ch & 0x3F) | 0x80);
      return true;
    }
    if(ch < 0x110000)
    {
      str.append((ch>>18) | 0xF0);
      str.append(((ch>>12) & 0x3F) | 0x80);
      str.append(((ch>>6) & 0x3F) | 0x80);
      str.append((ch & 0x3F) | 0x80);
      return true;
    }
    // todo: 5th byte
    // todo: 6th byte
#endif
    return false;
  }

  static bool_t append(const uint32_t* data, size_t size, String& str)
  {
    bool result = true;
    for(const uint32_t* end = data + size; data < end; ++data)
      result &= append(*data, str);
    return result;
  }

  static size_t length(tchar_t ch)
  {
#ifdef _UNICODE
    // todo
#else
    if((ch & 0x80) == 0)
      return 1;
    if((ch & 0xe0) == 0xc0)
      return 2;
    if((ch & 0xf0) == 0xe0)
      return 3;
    if((ch & 0xf8) == 0xf0)
      return 4;
    // todo: 5th byte
    // todo: 6th byte
#endif
    return 0;
  }

  static uint32_t fromString(const tchar_t* ch, size_t len)
  {
    uint32_t result = 0;
#ifdef _UNICODE
    // todo
#else
    if(len < 1)
      return 0;
    if(*(const uchar_t*)ch < 0x80)
      return *(const uchar_t*)ch;
    size_t reqLen = length(*ch);
    if(len < reqLen)
      return 0;
    static const uint32_t utf8Offsets[] = {0UL, 0UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL};
    switch(reqLen)
    {
      case 6: result = *(const uchar_t*)ch++ << 6;
      case 5: result += *(const uchar_t*)ch++; result <<= 6;
      case 4: result += *(const uchar_t*)ch++; result <<= 6;
      case 3: result += *(const uchar_t*)ch++; result <<= 6;
      case 2: result += *(const uchar_t*)ch++; result <<= 6;
      default: result += *(const uchar_t*)ch;
    }
    result -= utf8Offsets[reqLen];
#endif
    return result;
  }
  
  static uint32_t fromString(const String& str) {return fromString(str, str.length());}

  static bool_t isValid(const tchar_t* ch, size_t len)
  {
    // todo: check if ch[x] & 0x?0 == 0x80
    return len && len >= length(*ch);
  }
  
  static bool_t isValid(const String& str) {return isValid(str, str.length());}
};
