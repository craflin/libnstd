
#include <nstd/Error.hpp>
#include <nstd/File.hpp>
#include <nstd/Document/Xml.hpp>

Xml::Variant::NullData Xml::Variant::nullData;

class Xml::Private
{
public:
  class Position
  {
  public:
    int line;
    const tchar* pos;
    const tchar* lineStart;
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
  Position pos;

  int errorLine;
  int errorColumn;
  String errorString;

public:
  Private() : errorLine(0), errorColumn(0) {}

  bool parse(const tchar* data, Element& element);

  bool parseElement(Element& element);
  bool parseText(String& text);

  bool readToken();

  void skipSpace();

  void syntaxError(const Position& pos, const String& error);

public:
  static String unescapeString(const String& str);
  static String escapeString(const String& str);
  static String escapeStrings[5];
  static const tchar* escapeChars;
};

class Xml::Parser::Private : public Xml::Private
{
};

const tchar* Xml::Private::escapeChars = _T("'\"&<>");
String Xml::Private::escapeStrings[5] = {String(_T("apos")), String(_T("quot")), String(_T("amp")), String(_T("lt")), String(_T("gt"))};

bool Xml::Private::readToken()
{
  skipSpace();
  token.pos = pos;
  switch(*pos.pos)
  {
  case '<':
    if(pos.pos[1] == '/')
    {
      token.type = Token::endTagBeginType;
      pos.pos += 2;
      return true;
    }
    token.type = Token::startTagBeginType;
    ++pos.pos;
    return true;
  case '>':
    token.type = Token::tagEndType;
    ++pos.pos;
    return true;
  case '\0':
    token.type = Token::eofType;
    return true;
  case '=':
    token.type = Token::equalsSignType;
    ++pos.pos;
    return true;
  case '"':
  case '\'':
    {
      tchar endChars[4] = _T("x\r\n");
      *endChars = *pos.pos;
      const tchar* end = String::findOneOf(pos.pos + 1, endChars);
      if(!end)
        return syntaxError(pos, _T("Unexpected end of file")), false;
      if(*end != *pos.pos)
        return syntaxError(pos, _T("New line in string")), false;
      String escapedValue;
      escapedValue.attach(pos.pos + 1, end - pos.pos - 1);
      token.value = unescapeString(escapedValue);
      token.type = Token::stringType;
      pos.pos = end + 1;
      return true;
    }
  case '/':
    if(pos.pos[1] == '>')
    {
      token.type = Token::emptyTagEndType;
      pos.pos += 2;
      return true;
    }
    // no break
  default: // attribute or tag name
    {
      const tchar* end = pos.pos;
      while(*end && *end != '/' && *end != '>' && *end != '=' && !String::isSpace(*end))
        ++end;
      if(end == pos.pos)
        return syntaxError(pos, _T("Expected name")), false;
      token.value = String(pos.pos, end - pos.pos);
      token.type = Token::nameType;
      pos.pos = end;
      return true;
    }
  }
}

void Xml::Private::skipSpace()
{
  for(tchar c;;)
    switch((c = *pos.pos))
    {
    case '\r':
      if(*(++pos.pos) == '\n')
        ++pos.pos;
      ++pos.line;
      pos.lineStart = pos.pos;
      continue;
    case '\n':
      ++pos.line;
      ++pos.pos;
      pos.lineStart = pos.pos;
      continue;
    case '<':
      if(String::compare(pos.pos + 1, _T("!--"), 3) == 0)
      {
        pos.pos += 4;
        for(;;)
        {
          const tchar* end = String::findOneOf(pos.pos, _T("-\n\r"));
          if(!end)
          {
            pos.pos = pos.pos + String::length(pos.pos);
            return;
          }
          pos.pos = end;
          switch(*pos.pos)
          {
          case '\r':
            if(*(++pos.pos) == '\n')
              ++pos.pos;
            ++pos.line;
            pos.lineStart = pos.pos;
            continue;
          case '\n':
            ++pos.line;
            ++pos.pos;
            pos.lineStart = pos.pos;
            continue;
          default:
            if(String::compare(pos.pos + 1, _T("->"), 2) == 0)
            {
              pos.pos = end + 3;
              break;
            }
            ++pos.pos;
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

void Xml::Private::syntaxError(const Position& pos, const String& error)
{
  errorLine = pos.line;
  errorColumn = (int)(pos.pos - pos.lineStart) + 1;
  errorString = error;
}

String Xml::Private::unescapeString(const String& str)
{
  String result(str.length());
  tchar* destStart = result;
  tchar* dest = destStart;
  for(const tchar* i = str, * end = i + str.length(); i < end;)
    if(*i == '&')
    {
      ++i;
      const tchar* squenceEnd = String::find(i, _T(';'));
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

String Xml::Private::escapeString(const String& str)
{
  String result(str.length() + 200);
  tchar* destStart = result;
  tchar* dest = destStart;
  tchar c;
  for(const tchar* i = str, * end = i + str.length(); i < end; ++i)
  {
    c = *i;
    if((c & 0xc0) || (c & 0xe0) == 0) // c >= 64 || c < 32
    {
      *(dest++) = c;
      continue;
    }
    
    const tchar* escapeChar = String::find(escapeChars, c);
    if(escapeChar)
    {
      const String& escapeString = escapeStrings[escapeChar - escapeChars];
      result.resize(dest - destStart);
      result.reserve(result.length() + escapeString.length() + 1 + (end - i));
      destStart = result;
      dest = destStart + result.length();
      *(dest++) = '&';
      Memory::copy(dest, (const tchar*)escapeString, escapeString.length() * sizeof(tchar));
      dest += escapeString.length();
      *(dest++) = ';';
    }
    else
      *(dest++) = c;
  }
  result.resize(dest - destStart);
  return result;
}

bool Xml::Private::parse(const tchar* data, Element& element)
{
  pos.line = 1;
  pos.pos = pos.lineStart = data;

  skipSpace();
  if(*pos.pos == '<' && pos.pos[1] == '?')
  {
    Position pos = this->pos;
    for(;;)
    {
      const tchar* end = String::findOneOf(pos.pos, _T("\r\n?"));
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

bool Xml::Private::parseElement(Element& element)
{
  element.line = token.pos.line;
  element.column = (int)(token.pos.pos - token.pos.lineStart) + 1;
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

bool Xml::Private::parseText(String& text)
{
  const tchar* start = pos.pos;
  for(;;)
  {
    const tchar* end = String::findOneOf(pos.pos, _T("<\r\n"));
    if(!end)
    {
      pos.pos = pos.pos + String::length(pos.pos);
      return syntaxError(pos, _T("Unexpected end of file")), false;
    }
    pos.pos = end;
    switch(*pos.pos)
    {
    case '\r':
      if(*(++pos.pos) == '\n')
        ++pos.pos;
      ++pos.line;
      pos.lineStart = pos.pos;
      continue;
    case '\n':
      ++pos.line;
      ++pos.pos;
      pos.lineStart = pos.pos;
      continue;
    default:
      {
        String escapedText;
        escapedText.attach(start, pos.pos - start);
        text = unescapeString(escapedText);
      }
      return true;
    }
  }
}

Xml::Parser::Parser() : p(new Private) {}
Xml::Parser::~Parser() {delete p;}

int Xml::Parser::getErrorLine() const {return p->errorLine;}
int Xml::Parser::getErrorColumn() const {return p->errorColumn;}
String Xml::Parser::getErrorString() const {return p->errorString;}

bool Xml::Parser::parse(const String& data, Element& element) {return p->parse(data, element);}

bool Xml::Parser::load(const String& filePath, Element& element)
{
  File file;
  if(!file.open(filePath))
    return p->errorString = Error::getErrorString(), false;
  String data;
  if(!file.readAll(data))
    return p->errorString = Error::getErrorString(), false;
  return p->parse(data, element);
}

bool Xml::parse(const tchar* data, Element& element)
{
  Private parser;
  if(parser.parse(data, element))
    return true;
  Error::setErrorString(String::fromPrintf(_T("Syntax error at line %d, column %d: %s"), parser.errorLine, parser.errorColumn, (const tchar*)parser.errorString));
  return false;
}

bool Xml::parse(const String& data, Element& element)
{
  return parse((const tchar*)data, element);
}

bool Xml::load(const String& filePath, Element& element)
{
  File file;
  if(!file.open(filePath))
    return false;
  String data;
  if(!file.readAll(data))
    return false;
  return parse(data, element);
}

bool Xml::save(const Element& element, const String& filePath)
{
  File file;
  if(!file.open(filePath, File::writeFlag))
    return false;
  return file.write(toString(element));
}

String Xml::toString(const Element& element)
{
  String result(_T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"));
  result += element.toString();
  return result;
}

String Xml::Element::toString() const
{
  String result(type.length() + 200);
  result.append('<');
  result.append(type);
  for(HashMap<String, String>::Iterator i = attributes.begin(), end = attributes.end(); i != end; ++i)
  {
    result.append(' ');
    result.append(i.key());
    result.append(_T("=\""));
    result.append(Xml::Private::escapeString(*i));
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
        result.append(Xml::Private::escapeString(variant.toString()));
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
