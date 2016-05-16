
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

  Variant() : data2(&nullData) {}
  ~Variant() {clear();}

  Variant(const Variant& other)
  {
    if(other.data2->ref)
    {
      data2 = other.data2;
      Atomic::increment(data2->ref);
    }
    else switch(other.data2->type)
    {
      case nullType:
      case boolType:
        _data2.type = boolType;
        _data2.ref = 0;
        _data2.data.boolData = other.data2->data.boolData;
        data2 = &_data2;
        break;
      case intType:
      case uintType:
        _data2.type = other.data2->type;
        _data2.ref = 0;
        _data2.data.intData = other.data2->data.intData;
        data2 = &_data2;
        break;
      case doubleType:
      case int64Type:
      case uint64Type:
        _data2.type = other.data2->type;
        _data2.ref = 0;
        _data2.data.int64Data = other.data2->data.int64Data;
        data2 = &_data2;
        break;
#ifdef ASSERT
      case mapType:
      case listType:
      case stringType:
        ASSERT(false);
        // no break
#endif
      default:
        data2 = &nullData;
        break;
        break;
    }
  }

  Variant(bool_t val) : data2(&_data2) {_data2.type = boolType; _data2.ref = 0; _data2.data.boolData = val;}
  Variant(double val) : data2(&_data2) {_data2.type = doubleType; _data2.ref = 0; _data2.data.doubleData = val;}
  Variant(int_t val) : data2(&_data2) {_data2.type = intType; _data2.ref = 0; _data2.data.intData = val;}
  Variant(uint_t val) : data2(&_data2) {_data2.type = uintType; _data2.ref = 0; _data2.data.uintData = val;}
  Variant(int64_t val) : data2(&_data2) {_data2.type = int64Type; _data2.ref = 0; _data2.data.int64Data = val;}
  Variant(uint64_t val) : data2(&_data2) {_data2.type = uint64Type; _data2.ref = 0; _data2.data.uint64Data = val;}

  Variant(const HashMap<String, Variant>& val)
  {
    data2 = (Data*)Memory::alloc(sizeof(Data) + sizeof(HashMap<String, Variant>));
    HashMap<String, Variant>* map = (HashMap<String, Variant>*)(data2 + 1);
    new (map) HashMap<String, Variant>(val);
    data2->type = mapType;
    data2->ref = 1;
  }

  Variant(const List<Variant>& val)
  {
    data2 = (Data*)Memory::alloc(sizeof(Data) + sizeof(List<Variant>));
    List<Variant>* list = (List<Variant>*)(data2 + 1);
    new (list) List<Variant>(val);
    data2->type = listType;
    data2->ref = 1;
  }

  Variant(const String& val)
  {
    data2 = (Data*)Memory::alloc(sizeof(Data) + sizeof(String));
    String* string = (String*)(data2 + 1);
    new (string) String(val);
    data2->type = stringType;
    data2->ref = 1;
  }

  void_t clear()
  {
    if(data2->ref && Atomic::decrement(data2->ref) == 0)
    {
      switch(data2->type)
      {
      case mapType: ((HashMap<String, Variant>*)(data2 + 1))->~HashMap<String, Variant>(); break;
      case listType: ((List<Variant>*)(data2 + 1))->~List<Variant>(); break;
      case stringType: ((String*)(data2 + 1))->~String(); break;
      default: break;
      }
      Memory::free(data2);
    }
    data2 = &nullData;
  }

  Variant& operator=(const Variant& other)
  {
    clear();
    if(other.data2->ref)
    {
      data2 = other.data2;
      Atomic::increment(data2->ref);
    }
    else switch(other.data2->type)
    {
      case boolType:
        _data2.type = boolType;
        _data2.ref = 0;
        _data2.data.boolData = other.data2->data.boolData;
        data2 = &_data2;
        break;
      case intType:
      case uintType:
        _data2.type = other.data2->type;
        _data2.ref = 0;
        _data2.data.intData = other.data2->data.intData;
        data2 = &_data2;
        break;
      case doubleType:
      case int64Type:
      case uint64Type:
        _data2.type = other.data2->type;
        _data2.ref = 0;
        _data2.data.int64Data = other.data2->data.int64Data;
        data2 = &_data2;
        break;
#ifdef ASSERT
      case mapType:
      case listType:
      case stringType:
        ASSERT(false);
        // no break
#endif
      default:
        data2 = &nullData;
        break;
        break;
    }
    return *this;
  }

  Type getType() const {return data2->type;}

  bool_t isNull() const {return data2->type == nullType;}

  bool_t toBool() const
  {
    switch(data2->type)
    {
    case boolType: return data2->data.boolData;
    case doubleType: return data2->data.doubleData != 0.;
    case intType: return data2->data.intData != 0;
    case uintType: return data2->data.uintData != 0;
    case int64Type: return data2->data.int64Data != 0;
    case uint64Type: return data2->data.uint64Data != 0;
    case stringType:
      {
        const String& string = *(const String*)(data2 + 1);
        return string == _T("true") || string == _T("1");
      }
    default:
      return false;
    }
  }

  Variant& operator=(bool_t other)
  {
    if(data2->type != boolType)
    {
      clear();
      data2 = &_data2;
      _data2.type = boolType;
    }
    _data2.data.boolData = other;
    return *this;
  }

  double toDouble() const
  {
    switch(data2->type)
    {
    case boolType: return data2->data.boolData ? 1. : 0.;
    case doubleType: return data2->data.doubleData;
    case intType: return (double)data2->data.intData;
    case uintType: return (double)data2->data.uintData;
    case int64Type: return (double)data2->data.int64Data;
    case uint64Type: return (double)data2->data.uint64Data;
    case stringType: return ((const String*)(data2 + 1))->toDouble();
    default:
      return 0.;
    }
  }

  Variant& operator=(double other)
  {
    if(data2->type != doubleType)
    {
      clear();
      data2 = &_data2;
      _data2.type = doubleType;
    }
    _data2.data.doubleData = other;
    return *this;
  }

  int_t toInt() const
  {
    switch(data2->type)
    {
    case boolType: return data2->data.boolData ? 1 : 0;
    case doubleType: return (int_t)data2->data.doubleData;
    case intType: return data2->data.intData;
    case uintType: return (int_t)data2->data.uintData;
    case int64Type: return (int_t)data2->data.int64Data;
    case uint64Type: return (int_t)data2->data.uint64Data;
    case stringType: return ((const String*)(data2 + 1))->toInt();
    default:
      return 0;
    }
  }

  Variant& operator=(int_t other)
  {
    if(data2->type != intType)
    {
      clear();
      data2 = &_data2;
      _data2.type = intType;
    }
    _data2.data.intData = other;
    return *this;
  }

  int_t toUInt() const
  {
    switch(data2->type)
    {
    case boolType: return data2->data.boolData ? 1 : 0;
    case doubleType: return (uint_t)data2->data.doubleData;
    case intType: return (uint_t)data2->data.intData;
    case uintType: return data2->data.uintData;
    case int64Type: return (uint_t)data2->data.int64Data;
    case uint64Type: return (uint_t)data2->data.uint64Data;
    case stringType: return ((const String*)(data2 + 1))->toUInt();
    default:
      return 0;
    }
  }

  Variant& operator=(uint_t other)
  {
    if(data2->type != uintType)
    {
      clear();
      data2 = &_data2;
      _data2.type = uintType;
    }
    _data2.data.uintData = other;
    return *this;
  }

  int64_t toInt64() const
  {
    switch(data2->type)
    {
    case boolType: return data2->data.boolData ? 1 : 0;
    case doubleType: return (int64_t)data2->data.doubleData;
    case intType: return (int64_t)data2->data.intData;
    case uintType: return (int64_t)data2->data.uintData;
    case int64Type: return data2->data.int64Data;
    case uint64Type: return (int64_t)data2->data.uint64Data;
    case stringType: return ((const String*)(data2 + 1))->toInt64();
    default:
      return 0;
    }
  }

  Variant& operator=(int64_t other)
  {
    if(data2->type != int64Type)
    {
      clear();
      data2 = &_data2;
      _data2.type = int64Type;
    }
    _data2.data.int64Data = other;
    return *this;
  }

  uint64_t toUInt64() const
  {
    switch(data2->type)
    {
    case boolType: return data2->data.boolData ? 1 : 0;
    case doubleType: return (uint64_t)data2->data.doubleData;
    case intType: return (uint64_t)data2->data.intData;
    case uintType: return (uint64_t)data2->data.uintData;
    case int64Type: return (uint64_t)data2->data.int64Data;
    case uint64Type: return data2->data.uint64Data;
    case stringType: return ((const String*)(data2 + 1))->toUInt64();
    default:
      return 0;
    }
  }

  Variant& operator=(uint64_t other)
  {
    if(data2->type != uint64Type)
    {
      clear();
      data2 = &_data2;
      _data2.type = uint64Type;
    }
    _data2.data.uint64Data = other;
    return *this;
  }

  const HashMap<String, Variant>& toMap() const
  {
    if(data2->type == mapType)
      return *(const HashMap<String, Variant>*)(data2 + 1);
    static const HashMap<String, Variant> map;
    return map;
  }

  HashMap<String, Variant>& toMap()
  {
    if(data2->type != mapType)
    {
      clear();
      data2 = (Data*)Memory::alloc(sizeof(Data) + sizeof(HashMap<String, Variant>));
      HashMap<String, Variant>* map = (HashMap<String, Variant>*)(data2 + 1);
      new (map) HashMap<String, Variant>;
      data2->type = mapType;
      data2->ref = 1;
      return *map;
    }
    else if(data2->ref > 1)
    {
      Data* newData = (Data*)Memory::alloc(sizeof(Data) + sizeof(HashMap<String, Variant>));
      HashMap<String, Variant>* map = (HashMap<String, Variant>*)(data2 + 1);
      new (map) HashMap<String, Variant>(*(const HashMap<String, Variant>*)(data2 + 1));
      clear();
      data2 = newData;
      data2->type = mapType;
      data2->ref = 1;
      return *map;
    }
    return *(HashMap<String, Variant>*)(data2 + 1);
  }

  Variant& operator=(const HashMap<String, Variant>& other)
  {
    if(data2->type != mapType || data2->ref > 1)
    {
      clear();
      data2 = (Data*)Memory::alloc(sizeof(Data) + sizeof(HashMap<String, Variant>));
      HashMap<String, Variant>* map = (HashMap<String, Variant>*)(data2 + 1);
      new (map) HashMap<String, Variant>(other);
      data2->type = mapType;
      data2->ref = 1;
    }
    else
      *(HashMap<String, Variant>*)(data2 + 1) = other;
    return *this;
  }

  const List<Variant>& toList() const
  {
    if(data2->type == listType)
      return *(const List<Variant>*)(data2 + 1);
    static const List<Variant> list;
    return list;
  }

  List<Variant>& toList()
  {
    if(data2->type != listType)
    {
      clear();
      data2 = (Data*)Memory::alloc(sizeof(Data) + sizeof(List<Variant>));
      List<Variant>* list = (List<Variant>*)(data2 + 1);
      new (list) List<Variant>;
      data2->type = listType;
      data2->ref = 1;
      return *list;
    }
    else if(data2->ref > 1)
    {
      Data* newData = (Data*)Memory::alloc(sizeof(Data) + sizeof(List< Variant>));
      List<Variant>* list = (List<Variant>*)(data2 + 1);
      new (list) List<Variant>(*(const List<Variant>*)(data2 + 1));
      clear();
      data2 = newData;
      data2->type = listType;
      data2->ref = 1;
      return *list;
    }
    return *(List<Variant>*)(data2 + 1);
  }

  Variant& operator=(const List<Variant>& other)
  {
    if(data2->type != listType || data2->ref > 1)
    {
      clear();
      data2 = (Data*)Memory::alloc(sizeof(Data) + sizeof(List<Variant>));
      List< Variant>* list = (List<Variant>*)(data2 + 1);
      new (list) List<Variant>(other);
      data2->type = listType;
      data2->ref = 1;
    }
    else
      *(List<Variant>*)(data2 + 1) = other;
    return *this;
  }

  String toString() const
  {
    if(data2->type == stringType)
      return *(const String*)(data2 + 1);
    String string;
    switch(data2->type)
    {
    case boolType: string = data2->data.boolData ? String(_T("true")) : String(_T("false")); break;
    case doubleType: string.printf(_T("%f"), data2->data.doubleData); break;
    case intType: string.printf(_T("%d"), data2->data.intData); break;
    case uintType: string.printf(_T("%u"), data2->data.uintData); break;
    case int64Type: string.printf(_T("%lld"), data2->data.int64Data); break;
    case uint64Type: string.printf(_T("%llu"), data2->data.uint64Data); break;
    default: break;
    }
    return string;
  }

  Variant& operator=(const String& other)
  {
    if(data2->type != stringType || data2->ref > 1)
    {
      clear();
      data2 = (Data*)Memory::alloc(sizeof(Data) + sizeof(String));
      String* string = (String*)(data2 + 1);
      new (string) String(other);
      data2->type = stringType;
      data2->ref = 1;
    }
    else
      *(String*)(data2 + 1) = other;
    return *this;
  }

  void_t swap(Variant& other)
  {
    Variant tmp = other;
    other = *this;
    *this = tmp;
  }

private:
  struct Data
  {
    Type type;
    union Data2
    {
      int_t intData;
      uint_t uintData;
      double doubleData;
      bool_t boolData;
      int64_t int64Data;
      uint64_t uint64Data;
    } data;
    volatile size_t ref;
  };

  static struct NullData : public Data
  {
    NullData()
    {
      type = nullType;
      ref = 0;
    }
  } nullData;

  Data* data2;
  Data _data2;
};
