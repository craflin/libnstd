
#pragma once

#include <nstd/String.hpp>
#include <nstd/HashMap.hpp>
#include <nstd/List.hpp>

class Xml
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

    void clear()
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
    bool isNull() const {return data->type == nullType;}

    bool isText() const {return data->type == textType;}

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

    bool isElement() const {return data->type == elementType;}

    const Element& toElement() const
    {
      if(data->type == elementType)
        return *(const Element*)(data + 1);
      static const Element element = Element();
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
      volatile usize ref;
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
    int line;
    int column;
    String type;
    HashMap<String, String> attributes;
    List<Variant> content;

  public:
    void clear()
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

    int getErrorLine() const;
    int getErrorColumn() const;
    String getErrorString() const;

    bool parse(const tchar* data, Element& element);
    bool parse(const String& data, Element& element);

    bool load(const String& file, Element& element);

  private:
    class Private;
    Private* p;
  };

public:
  static bool parse(const tchar* data, Element& element);
  static bool parse(const String& data, Element& element);

  static bool load(const String& file, Element& element);
  static bool save(const Element& element, const String& file);

  static String toString(const Element& element);

private:
  class Private;
};
