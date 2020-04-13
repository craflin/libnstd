
#pragma once

#include <nstd/String.hpp>

class Unicode
{
public:
  static String toString(uint32 ch)
  {
#ifdef _UNICODE
    String result(2);
#else
    String result(4);
#endif
    append(ch, result);
    return result;
  }

  static String toString(const uint32* data, usize size)
  {
      String result(size + 200);
      append(data, size, result);
      return result;
  }

  static bool append(uint32 ch, String& str)
  {
#ifdef _UNICODE
    if((ch & ~(0x10000UL - 1)) == 0) // ch < 0x10000
    {
      if((ch & 0xF800ULL) != 0xD800ULL) // ch < 0xD800 || ch > 0xDFFF
      {
        str.append((tchar)ch);
        return true;
      }
      return false;
    }
    if(ch < 0x110000UL)
    {
      ch -= 0x10000UL;
      str.append((tchar)((ch >> 10) | 0xD800UL));
      str.append((ch & 0x3ffULL) | 0xDC00UL);
      return true;
    }
#else
    if((ch & ~(0x80UL - 1)) == 0) // ch < 0x80
    {
      str.append((char)ch);
      return true;
    }
    if((ch & ~(0x800UL - 1)) == 0) // ch < 0x800
    {
      str.append((ch >> 6) | 0xC0);
      str.append((ch & 0x3F) | 0x80);
      return true;
    }
    if((ch & ~(0x10000UL - 1)) == 0) // ch < 0x10000
    {
      str.append((ch >> 12) | 0xE0);
      str.append(((ch >> 6) & 0x3F) | 0x80);
      str.append((ch & 0x3F) | 0x80);
      return true;
    }
    if(ch < 0x110000UL)
    {
      str.append((ch >> 18) | 0xF0);
      str.append(((ch >> 12) & 0x3F) | 0x80);
      str.append(((ch >> 6) & 0x3F) | 0x80);
      str.append((ch & 0x3F) | 0x80);
      return true;
    }
#endif
    return false;
  }

  static bool append(const uint32* data, usize size, String& str)
  {
    bool result = true;
    for(const uint32* end = data + size; data < end; ++data)
      result &= append(*data, str);
    return result;
  }

  static usize length(tchar ch)
  {
#ifdef _UNICODE
    if((ch & 0xF800ULL) != 0xD800ULL) // ch < 0xD800 || ch > 0xDFFF
      return 1;
    if((ch & 0xFC00ULL) == 0xD800ULL) // ch <= 0xDBFF
      return 2;
#else
    if((ch & 0x80) == 0)
      return 1;
    if((ch & 0xe0) == 0xc0)
      return 2;
    if((ch & 0xf0) == 0xe0)
      return 3;
    if((ch & 0xf8) == 0xf0)
      return 4;
#endif
    return 0;
  }

  static uint32 fromString(const tchar* ch, usize len)
  {
    if(len == 0)
      return 0;
#ifdef _UNICODE
   if((*ch & 0xF800ULL) != 0xD800ULL) // ch < 0xD800 || ch > 0xDFFF
     return *(const uint16*)ch;
    if((*ch & 0xFC00ULL) == 0xD800ULL) // ch <= 0xDBFF
    {
     if(len < 2)
       return 0;
      return (ch[1] & 0x3FFULL | ((uint32)(*ch & 0x3FFULL) << 10)) + 0x10000UL;
    }
    else
      return *(const uint16*)ch;
#else
    if((*(const uchar*)ch & 0x80) == 0) // ch < 0x80
      return *(const uchar*)ch;
    usize reqLen = length(*ch);
    if(len < reqLen)
      return 0;
    static const uint32 utf8Offsets[] = {0UL, 0UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL};
    uint32 result = 0;
    switch(reqLen)
    {
      case 4: result = (uint32)*(const uchar*)ch++ << 6;
      case 3: result += (uint32)*(const uchar*)ch++; result <<= 6;
      case 2: result += (uint32)*(const uchar*)ch++; result <<= 6;
      default: result += *(const uchar*)ch;
    }
    result -= utf8Offsets[reqLen];
    return result;
#endif
  }
  
  static uint32 fromString(const String& str) {return fromString(str, str.length());}

  static bool isValid(const tchar* ch, usize len)
  {
      for(const tchar* end = ch + len; ch < end;)
      {
#ifdef _UNICODE
        if((*ch & 0xF800ULL) != 0xD800ULL) // ch < 0xD800 || ch > 0xDFFF
        {
          ++ch;
          --len;
          continue;
        }
        if(len > 1 && (*ch & 0xFC00ULL) == 0xD800ULL) // ch <= 0xDBFF
          if((ch[1] & 0xFC00UL) == 0xDC00UL)
          {
            ch += 2;
            len -= 2;
            continue;
          }
        return false;
#else
        usize minLen = length(*ch);
        if(len < minLen)
          return false;
        switch(minLen)
        {
        case 4:
          if(((((const uchar*)ch)[1] | ((uint32)((const uchar*)ch)[2] << 8) | ((uint32)((const uchar*)ch)[3] << 16)) & 0xc0c0c0UL) != 0x808080UL)
            return false;
          break;
        case 3:
          if(((((const uchar*)ch)[1] | ((uint32)((const uchar*)ch)[2] << 8)) & 0xc0c0UL) != 0x8080UL)
            return false;
          break;
        case 2:
            if((ch[1] & 0xc0) != 0x80)
              return false;
            break;
        case 1:
          break;
        default:
          return false;
        }
        ch += minLen;
        len -= minLen;
#endif
      }
    return true;
  }
  
  static bool isValid(const String& str) {return isValid(str, str.length());}
};
