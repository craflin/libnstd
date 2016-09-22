
#pragma once

#include <nstd/String.h>
#include <nstd/HashMap.h>
#include <nstd/List.h>

class XML
{
public:
  class Element;

  class Variant
  {
  public:
    enum Type
    {
      nullType,
      elementType,
      textType,
    };

  public:

    Variant() : data(&nullData) {}
    ~Variant() {clear();}

    Variant(const Variant& other)
    {
      if(other.data->ref)
      {
        data = other.data;
        Atomic::increment(data->ref);
      }
      else
        data = &nullData;
    }

    Variant(const Element& val)
    {
      data = (Data*)Memory::alloc(sizeof(Data) + sizeof(Element));
      Element* element = (Element*)(data + 1);
      new (element) Element(val);
      data->type = elementType;
      data->ref = 1;
    }

    Variant(const String& val)
    {
      data = (Data*)Memory::alloc(sizeof(Data) + sizeof(String));
      String* string = (String*)(data + 1);
      new (string) String(val);
      data->type = textType;
      data->ref = 1;
    }

    void_t clear()
    {
      if(data->ref && Atomic::decrement(data->ref) == 0)
      {
        switch(data->type)
        {
        case elementType: ((Element*)(data + 1))->~Element(); break;
        case textType: ((String*)(data + 1))->~String(); break;
        default: break;
        }
        Memory::free(data);
      }
      data = &nullData;
    }

    Type getType() const {return data->type;}
    bool_t isNull() const {return data->type == nullType;}

    bool_t isText() const {return data->type == textType;}

    String toString() const
    {
      if(data->type == textType)
        return *(const String*)(data + 1);
      return String();
    }

    Variant& operator=(const String& other)
    {
      if(data->type != textType || data->ref > 1)
      {
        clear();
        data = (Data*)Memory::alloc(sizeof(Data) + sizeof(String));
        String* string = (String*)(data + 1);
        new (string) String(other);
        data->type = textType;
        data->ref = 1;
      }
      else
        *(String*)(data + 1) = other;
      return *this;
    }

    bool_t isElement() const {return data->type == elementType;}

    const Element& toElement() const
    {
      if(data->type == elementType)
        return *(const Element*)(data + 1);
      static const Element element;
      return element;
    }

    Element& toElement()
    {
      if(data->type != elementType)
      {
        clear();
        data = (Data*)Memory::alloc(sizeof(Data) + sizeof(Element));
        Element* element = (Element*)(data + 1);
        new (element) Element;
        data->type = elementType;
        data->ref = 1;
        return *element;
      }
      else if(data->ref > 1)
      {
        Data* newData = (Data*)Memory::alloc(sizeof(Data) + sizeof(Element));
        Element* element = (Element*)(data + 1);
        new (element) Element(*(const Element*)(data + 1));
        clear();
        data = newData;
        data->type = elementType;
        data->ref = 1;
        return *element;
      }
      return *(Element*)(data + 1);
    }

  private:
    struct Data
    {
      Type type;
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

    Data* data;
    Data _data;
  };

  class Element
  {
  public:
    int_t line;
    int_t column;
    String type;
    HashMap<String, String> attributes;
    List<Variant> content;

  public:
    void_t clear()
    {
      type.clear();
      attributes.clear();
      content.clear();
    }

    String toString() const;
  };

  class Parser
  {
  public:
    Parser();
    ~Parser();

    int_t getErrorLine() const;
    int_t getErrorColumn() const;
    String getErrorString() const;

    bool_t parse(const tchar_t* data, Element& element);
    bool_t parse(const String& data, Element& element);

    bool_t load(const String& file, Element& element);

  private:
    class Private;
    Private* p;
  };

public:
  static bool_t parse(const tchar_t* data, Element& element);
  static bool_t parse(const String& data, Element& element);

  static bool_t load(const String& file, Element& element);
  static bool_t save(const Element& element, const String& file);

  static String toString(const Element& element);

private:
  class Private;
};
