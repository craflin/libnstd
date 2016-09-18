
#include <nstd/Error.h>
#include <nstd/File.h>
#include <nstd/XML/XML.h>

XML::Variant::NullData XML::Variant::nullData;

class XML::Parser::Private
{
public:
  class Position
  {
  public:
    int_t line;
    const tchar_t* pos;
  };

  class Token
  {
  public:
    enum Type
    {
      startTagBeginType, // <
      tagEndType, // >
      endTagBeginType, // </
      emptyTagEndType, // />
      equalsSignType, // =
      stringType,
      nameType, // attribute or tag name
      eofType,
    };

  public:
    Type type;
    String value;
    Position pos;
  };

public:
  Token token;
  const tchar_t* start;
  Position pos;

  

  int_t errorLine;
  int_t errorColumn;
  String errorString;

  bool_t parse(const String& data, Element& element);
  bool_t parseElement(Element& element);
  bool_t parseText(String& text);

  bool_t readToken();

  void_t skipSpace();

  void_t syntaxError(const Position& pos, const String& error);
};

class XML::Private
{
public:
  static String unescapeString(const String& str);
  static String escapeString(const String& str);
  static String escapeStrings[5];
  static const tchar_t* escapeChars;
};

const tchar_t* XML::Private::escapeChars = _T("'\"&<>");
String XML::Private::escapeStrings[5] = {String(_T("apos")), String(_T("quot")), String(_T("amp")), String(_T("lt")), String(_T("gt"))};

bool_t XML::Parser::Private::readToken()
{
  skipSpace();
  switch(*pos.pos)
  {
  case '<':
    if(pos.pos[1] == '/')
    {
      token.pos = pos;
      token.type = Token::endTagBeginType;
      pos.pos += 2;
      return true;
    }
    token.pos = pos;
    token.type = Token::startTagBeginType;
    ++pos.pos;
    return true;
  case '>':
    token.pos = pos;
    token.type = Token::tagEndType;
    ++pos.pos;
    return true;
  case '\0':
    token.pos = pos;
    token.type = Token::eofType;
    return true;
  case '=':
    token.pos = pos;
    token.type = Token::equalsSignType;
    ++pos.pos;
    return true;
  case '"':
  case '\'':
    {
      tchar_t endChars[4] = _T("x\r\n");
      *endChars = *pos.pos;
      const tchar_t* end = String::findOneOf(pos.pos + 1, endChars);
      if(!end)
        return syntaxError(pos, _T("Unexpected end of file")), false;
      if(*end != *pos.pos)
        return syntaxError(pos, _T("New line in string")), false;
      token.pos = pos;
      String escapedValue;
      escapedValue.attach(pos.pos + 1, end - pos.pos - 1);
      token.value = XML::Private::unescapeString(escapedValue);
      token.type = Token::stringType;
      pos.pos = end + 1;
      return true;
    }
  case '/':
    if(pos.pos[1] == '>')
    {
      token.pos = pos;
      token.type = Token::emptyTagEndType;
      pos.pos += 2;
    return true;
    }
    // no break
  default: // attribute or tag name
    {
      const tchar_t* end = pos.pos;
      while(*end && *end != '/' && *end != '>' && *end != '=' && !String::isSpace(*end))
        ++end;
      if(end == pos.pos)
        return syntaxError(pos, _T("Expected name")), false;
      token.pos = pos;
      token.value = String(pos.pos, end - pos.pos);
      token.type = Token::nameType;
      pos.pos = end;
      return true;
    }
  }
}

void_t XML::Parser::Private::skipSpace()
{
  for(tchar_t c;;)
    switch((c = *pos.pos))
    {
    case '\r':
      if(*(++pos.pos) == '\n')
        ++pos.pos;
      ++pos.line;
      continue;
    case '\n':
      ++pos.line;
      ++pos.pos;
      continue;
    case '<':
      if(String::compare(pos.pos + 1, _T("!--")) == 0)
      {
        const tchar_t* end = pos.pos + 4;
        for(;;)
        {
          int_t line = pos.line;
          const tchar_t* newEnd = String::findOneOf(end, _T("-\n\r"));
          if(!newEnd)
          {
            pos.pos = end + String::length(end);
            pos.line = line;
            return;
          }
          end = newEnd;
          switch(*newEnd)
          {
          case '\r':
            if(*(++end) == '\n')
              ++end;
            ++line;
            continue;
          case '\n':
            ++line;
            ++end;
            continue;
          default:
            if(String::compare(end + 1, _T("->")) == 0)
            {
              pos.pos = end + 3;
              pos.line = line;
              break;
            }
            ++end;
            continue;
          }
          break;
        }
        continue;
      }
    default:
      if(String::isSpace(c))
        ++pos.pos;
      else
        return;
    }
}

void_t XML::Parser::Private::syntaxError(const Position& pos, const String& error)
{
  int column = 1;
  for(const tchar_t* p = pos.pos; p > start;)
  {
    --p;
    if(*p == '\n' || *p == '\r')
      break;
    ++column;
  }
  errorLine = pos.line;
  errorColumn = column;
  errorString = error;
}

String XML::Private::unescapeString(const String& str)
{
  String result(str.length());
  tchar_t* destStart = result;
  tchar_t* dest = destStart;
  for(const tchar_t* i = str, * end = i + str.length(); i < end;)
    if(*i == '&')
    {
      ++i;
      const tchar_t* squenceEnd = String::find(i, _T(';'));
      if(squenceEnd)
      {
        String str;
        str.attach(i, squenceEnd - i);
        for(String* j = escapeStrings, * end = escapeStrings + sizeof(escapeStrings) / sizeof(*escapeStrings); j < end; ++j)
            if(str == *j)
            {
                *(dest++) = escapeChars[j - escapeStrings];
                i = squenceEnd + 1;
                goto translated;
            }
        *(dest++) = *(i++);
    translated: ;
        // todo: handle numeric stuff like &11111; &xffff;
      }
      else
        *(dest++) = *(i++);
    }
    else
      *(dest++) = *(i++);
  result.resize(dest - destStart);
  return result;
}

String XML::Private::escapeString(const String& str)
{
  String result(str.length() + 200);
  tchar_t* destStart = result;
  tchar_t* dest = destStart;
  tchar_t c;
  for(const tchar_t* i = str, * end = i + str.length(); i < end; ++i)
  {
    c = *i;
    if((c & 0xc0) || (c & 0xe0) == 0) // c >= 64 || c < 32
    {
      *(dest++) = c;
      continue;
    }
    
    const tchar_t* escapeChar = String::find(escapeChars, c);
    if(escapeChar)
    {
      const String& escapeString = escapeStrings[escapeChar - escapeChars];
      result.resize(dest - destStart);
      result.reserve(result.length() + escapeString.length() + 1 + (end - i));
      destStart = result;
      dest = destStart + result.length();
      *(dest++) = '&';
      Memory::copy(dest, (const tchar_t*)escapeString, escapeString.length() * sizeof(tchar_t));
      dest += escapeString.length();
      *(dest++) = ';';
    }
    else
      *(dest++) = c;
  }
  result.resize(dest - destStart);
  return result;
}

bool_t XML::Parser::Private::parse(const String& data, Element& element)
{
  start = data;
  pos.line = 1;
  pos.pos = start;

  skipSpace();
  if(*pos.pos == '<' && pos.pos[1] == '?')
  {
    Position pos = this->pos;
    for(;;)
    {
      const tchar_t* end = String::findOneOf(pos.pos, _T("\r\n?"));
      if(!end)
        return this->syntaxError(pos, _T("Unexpected end of file")), false;
      if(*end == '?' && end[1] == '>')
      {
        pos.pos = end + 2;
        break;
      }
      pos.pos = end;
      skipSpace();
    }
  }
  if(!readToken())
    return false;
  if(token.type != Token::startTagBeginType)
    return syntaxError(token.pos, _T("Expected '<'")), false;
  return parseElement(element);
}

bool_t XML::Parser::Private::parseElement(Element& element)
{
  if(!readToken())
    return false;
  if(token.type != Token::nameType)
    return syntaxError(token.pos, _T("Expected tag name")), false;
  element.type = token.value;
  for(;;)
  {
    if(!readToken())
      return false;
    if(token.type == Token::emptyTagEndType)
      return true;
    if(token.type == Token::tagEndType)
      break;
    if(token.type == Token::nameType)
    {
      String attributeName = token.value;
      if(!readToken())
        return false;
      if(token.type != Token::equalsSignType)
        return syntaxError(token.pos, _T("Expected '='")), false;
      if(!readToken())
        return false;
      if(token.type != Token::stringType)
        return syntaxError(token.pos, _T("Expected string")), false;
      element.attributes.append(attributeName, token.value);
      continue;
    }
  }
  for(;;)
  {
    Position pos = this->pos;
    if(readToken())
    {
      if(token.type == Token::endTagBeginType)
        break;
      if(token.type == Token::startTagBeginType)
      {
        Variant& variant = element.content.append(Variant());
        if(!parseElement(variant.toElement()))
          return false;
        continue;
      }
      else
        this->pos = pos;
    }
    String string;
    if(!parseText(string))
      return false;
    element.content.append(string);
  }
  if(!readToken())
    return false;
  if(token.type != Token::nameType)
    return syntaxError(token.pos, _T("Expected tag name")), false;
  if(token.value != element.type)
    return syntaxError(token.pos, String(_T("Expected end tag of '")) + element.type + _T("'")), false;
  if(!readToken())
    return false;
  if(token.type != Token::tagEndType)
    return syntaxError(token.pos, _T("Expected '>'")), false;
  return true;
}

bool_t XML::Parser::Private::parseText(String& text)
{
  const tchar_t* start = pos.pos;
  int_t line = pos.line;
  const tchar_t* end = pos.pos;
  for(;;)
  {
    const tchar_t* newEnd = String::findOneOf(end, _T("<\r\n"));
    if(!newEnd)
    {
      pos.pos = end + String::length(end);
      return syntaxError(pos, _T("Unexpected end of file")), false;
    }
    end = newEnd;
    switch(*end)
    {
    case '\r':
      if(*(++end) == '\n')
        ++end;
      ++line;
      continue;
    case '\n':
      ++line;
      ++end;
      continue;
    default:
      pos.pos = end;
      pos.line = line;
      {
        String escapedText;
        escapedText.attach(start, end - start);
        text = XML::Private::unescapeString(escapedText);
      }
      return true;
    }
  }
}

XML::Parser::Parser() : p(new Private) {}
XML::Parser::~Parser() {delete p;}

int_t XML::Parser::getErrorLine() const {return p->errorLine;}
int_t XML::Parser::getErrorColumn() const {return p->errorColumn;}
String XML::Parser::getErrorString() const {return p->errorString;}

bool_t XML::Parser::parse(const String& data, Element& element) {return p->parse(data, element);}

bool_t XML::parse(const String& data, Element& element)
{
  Parser parser;
  if(parser.parse(data, element))
    return true;
  Error::setErrorString(String::fromPrintf(_T("Syntax error at line %d, column %d: %s"), parser.getErrorLine(), parser.getErrorColumn(), (const tchar_t*)parser.getErrorString()));
  return false;
}

bool_t XML::load(const String& filePath, Element& element)
{
  File file;
  if(!file.open(filePath))
    return false;
  String data;
  if(!file.readAll(data))
    return false;
  return parse(data, element);
}

bool_t XML::save(const Element& element, const String& filePath)
{
  File file;
  if(!file.open(filePath, File::writeFlag))
    return false;
  return file.write(toString(element));
}

String XML::toString(const Element& element)
{
  String result(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"));
  result += element.toString();
  return result;
}

String XML::Element::toString() const
{
  String result(type.length() + 200);
  result.append('<');
  result.append(type);
  for(HashMap<String, String>::Iterator i = attributes.begin(), end = attributes.end(); i != end; ++i)
  {
    result.append(' ');
    result.append(i.key());
    result.append(_T("=\""));
    result.append(XML::Private::escapeString(*i));
    result.append('"');
  }
  if(content.isEmpty())
    result.append(_T("/>"));
  else
  {
    result.append('>');
    for(List<Variant>::Iterator i = content.begin(), end = content.end(); i != end; ++i)
    {
      const Variant& variant = *i;
      switch(variant.getType())
      {
      case Variant::elementType:
        result.append(variant.toElement().toString());
        break;
      case Variant::textType:
        result.append(XML::Private::escapeString(variant.toString()));
        break;
      default: ;
      }
    }
    result.append(_T("</"));
    result.append(type);
    result.append('>');
  }
  return result;
}
