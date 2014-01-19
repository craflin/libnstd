
#pragma once

#include <nstd/String.h>
#include <nstd/HashMap.h>
#include <nstd/List.h>

class Variant
{
public:
  enum Type
  {
    nullType,
    boolType,
    doubleType,
    intType,
    uintType,
    int64Type,
    uint64Type,
    mapType,
    listType,
    stringType,
  };

  Variant() : type(nullType) {data.data = 0;}
  ~Variant() {clear();}

  Variant(const Variant& other) : type(other.type)
  {
    switch(other.type)
    {
    case nullType: data.data = 0; break;
    case boolType: data.boolData = other.data.boolData; break;
    case doubleType: data.doubleData = other.data.doubleData; break;
    case intType: data.intData = other.data.intData; break;
    case uintType: data.uintData = other.data.uintData; break;
    case int64Type: data.int64Data = other.data.int64Data; break;
    case uint64Type: data.uint64Data = other.data.uint64Data; break;
    case mapType: data.data = new HashMap<String, Variant>(*(const HashMap<String, Variant>*)other.data.data); break;
    case listType: data.data = new List<Variant>(*(const List<Variant>*)other.data.data); break;
    case stringType: data.data = new String(*(const String*)other.data.data); break;
    }
  }

  Variant(bool_t val) : type(boolType) {data.boolData = val;}
  Variant(double val) : type(doubleType) {data.doubleData = val;}
  Variant(int_t val) : type(intType) {data.intData = val;}
  Variant(uint_t val) : type(uintType) {data.uintData = val;}
  Variant(int64_t val) : type(int64Type) {data.int64Data = val;}
  Variant(uint64_t val) : type(uint64Type) {data.uint64Data = val;}
  Variant(const HashMap<String, Variant>& val) : type(mapType) {data.data = new HashMap<String, Variant>(val);}
  Variant(const List<Variant>& val) : type(listType) {data.data = new List<Variant>(val);}
  Variant(const String& val) : type(stringType) {data.data = new String(val);}

  void_t clear()
  {
    switch(type)
    {
    case mapType: delete (HashMap<String, Variant>*)data.data; break;
    case listType: delete (List<Variant>*)data.data; break;
    case stringType: delete (String*)data.data; break;
    default: break;
    }
    type = nullType;
    data.data = 0;
  }

  Variant& operator=(const Variant& other)
  {
    clear();
    type = other.type;
    switch(other.type)
    {
    case nullType: data.data = 0; break;
    case boolType: data.boolData = other.data.boolData; break;
    case doubleType: data.doubleData = other.data.doubleData; break;
    case intType: data.intData = other.data.intData; break;
    case uintType: data.uintData = other.data.uintData; break;
    case int64Type: data.int64Data = other.data.int64Data; break;
    case uint64Type: data.uint64Data = other.data.uint64Data; break;
    case mapType: data.data = new HashMap<String, Variant>(*(const HashMap<String, Variant>*)other.data.data); break;
    case listType: data.data = new List<Variant>(*(const List<Variant>*)other.data.data); break;
    case stringType: data.data = new String(*(const String*)other.data.data); break;
    }
    return *this;
  }

  Type getType() const {return type;}

  bool_t isNull() const {return type == nullType;}

  bool_t toBool() const
  {
    switch(type)
    {
    case boolType: return data.boolData;
    case doubleType: return data.doubleData != 0.;
    case intType: return data.intData != 0;
    case uintType: return data.uintData != 0;
    case int64Type: return data.int64Data != 0;
    case uint64Type: return data.uint64Data != 0;
    case stringType:
      {
        const String& string = *(const String*)data.data;
        return string == _T("true") || string == _T("1");
      }
    default:
      return false;
    }
  }

  Variant& operator=(bool_t other)
  {
    if(type != boolType)
    {
      clear();
      type = boolType;
    }
    data.boolData = other;
    return *this;
  }

  double toDouble() const
  {
    switch(type)
    {
    case boolType: return data.boolData ? 1. : 0.;
    case doubleType: return data.doubleData;
    case intType: return (double)data.intData;
    case uintType: return (double)data.uintData;
    case int64Type: return (double)data.int64Data;
    case uint64Type: return (double)data.uint64Data;
    case stringType: return ((const String*)data.data)->toDouble();
    default:
      return 0.;
    }
  }

  Variant& operator=(double other)
  {
    if(type != doubleType)
    {
      clear();
      type = doubleType;
    }
    data.doubleData = other;
    return *this;
  }

  int_t toInt() const
  {
    switch(type)
    {
    case boolType: return data.boolData ? 1 : 0;
    case doubleType: return (int_t)data.doubleData;
    case intType: return data.intData;
    case uintType: return (int_t)data.uintData;
    case int64Type: return (int_t)data.int64Data;
    case uint64Type: return (int_t)data.uint64Data;
    case stringType: return ((const String*)data.data)->toInt();
    default:
      return 0;
    }
  }

  Variant& operator=(int_t other)
  {
    if(type != intType)
    {
      clear();
      type = intType;
    }
    data.intData = other;
    return *this;
  }

  int_t toUInt() const
  {
    switch(type)
    {
    case boolType: return data.boolData ? 1 : 0;
    case doubleType: return (uint_t)data.doubleData;
    case intType: return (uint_t)data.intData;
    case uintType: return data.uintData;
    case int64Type: return (uint_t)data.int64Data;
    case uint64Type: return (uint_t)data.uint64Data;
    case stringType: return ((const String*)data.data)->toUInt();
    default:
      return 0;
    }
  }

  Variant& operator=(uint_t other)
  {
    if(type != uintType)
    {
      clear();
      type = uintType;
    }
    data.uintData = other;
    return *this;
  }

  int64_t toInt64() const
  {
    switch(type)
    {
    case boolType: return data.boolData ? 1 : 0;
    case doubleType: return (int64_t)data.doubleData;
    case intType: return (int64_t)data.intData;
    case uintType: return (int64_t)data.uintData;
    case int64Type: return data.int64Data;
    case uint64Type: return (int64_t)data.uint64Data;
    case stringType: return ((const String*)data.data)->toInt64();
    default:
      return 0;
    }
  }

  Variant& operator=(int64_t other)
  {
    if(type != int64Type)
    {
      clear();
      type = int64Type;
    }
    data.int64Data = other;
    return *this;
  }

  uint64_t toUInt64() const
  {
    switch(type)
    {
    case boolType: return data.boolData ? 1 : 0;
    case doubleType: return (uint64_t)data.doubleData;
    case intType: return (uint64_t)data.intData;
    case uintType: return (uint64_t)data.uintData;
    case int64Type: return (uint64_t)data.int64Data;
    case uint64Type: return data.uint64Data;
    case stringType: return ((const String*)data.data)->toUInt64();
    default:
      return 0;
    }
  }

  Variant& operator=(uint64_t other)
  {
    if(type != uint64Type)
    {
      clear();
      type = uint64Type;
    }
    data.uint64Data = other;
    return *this;
  }

  const HashMap<String, Variant>& toMap() const
  {
    if(type == mapType)
      return *(const HashMap<String, Variant>*)data.data;
    static const HashMap<String, Variant> map;
    return map;
  }

  HashMap<String, Variant>& toMap()
  {
    if(type != mapType)
    {
      clear();
      type = mapType;
      data.data = new HashMap<String, Variant>();
    }
    return *(HashMap<String, Variant>*)data.data;
  }

  Variant& operator=(const HashMap<String, Variant>& other)
  {
    if(type != mapType)
    {
      clear();
      type = mapType;
      data.data = new HashMap<String, Variant>(other);
    }
    else
      *(HashMap<String, Variant>*)data.data = other;
    return *this;
  }

  const List<Variant>& toList() const
  {
    if(type == listType)
      return *(const List<Variant>*)data.data;
    static const List<Variant> list;
    return list;
  }

  List<Variant>& toList()
  {
    if(type != listType)
    {
      clear();
      type = listType;
      data.data = new List<Variant>();
    }
    return *(List<Variant>*)data.data;
  }

  Variant& operator=(const List<Variant>& other)
  {
    if(type != listType)
    {
      clear();
      type = listType;
      data.data = new List<Variant>(other);
    }
    else
      *(List<Variant>*)data.data = other;
    return *this;
  }

  String toString() const
  {
    if(type == stringType)
      return *(const String*)data.data;
    String string;
    switch(type)
    {
    case boolType: if(data.boolData) string = _T("true"); else string = _T("false"); break;
    case doubleType: string.printf(_T("%f"), data.doubleData); break;
    case intType: string.printf(_T("%d"), data.intData); break;
    case uintType: string.printf(_T("%u"), data.uintData); break;
    case int64Type: string.printf(_T("%lld"), data.int64Data); break;
    case uint64Type: string.printf(_T("%llu"), data.uint64Data); break;
    default: break;
    }
    return string;
  }

  Variant& operator=(const String& other)
  {
    if(type != stringType)
    {
      clear();
      type = stringType;
      data.data = new String(other);
    }
    else
      *(String*)data.data = other;
    return *this;
  }

  void_t swap(Variant& other)
  {
    Type tmpType = type;
    Data tmpData = data;
    type = other.type;
    data = other.data;
    other.type = tmpType;
    other.data = tmpData;
  }

private:
  Type type;
  union Data
  {
    int_t intData;
    uint_t uintData;
    double doubleData;
    bool_t boolData;
    int64_t int64Data;
    uint64_t uint64Data;
    void_t* data;
  } data;
};
