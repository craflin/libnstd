
#include <nstd/Error.hpp>
#include <nstd/File.hpp>
#include <nstd/Document/Xml.hpp>
#include <nstd/Unicode.hpp>

Xml::Variant::NullData Xml::Variant::nullData;

class Xml::Private
{
public:
  class Position
  {
  public:
    int line;
    const char* pos;
    const char* lineStart;
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

  bool parse(const char* data, Element& element);

  bool parseElement(Element& element);
  bool parseText(String& text);

  bool readToken();

  void skipSpace();

  void syntaxError(const Position& pos, const String& error);

public:
  static String unescapeString(const String& str);
  static String escapeString(const String& str);
  static String escapeStrings[5];
  static const char* escapeChars;
};

class Xml::Parser::Private : public Xml::Private
{
};

const char* Xml::Private::escapeChars = "'\"&<>";
String Xml::Private::escapeStrings[5] = {String("apos"), String("quot"), String("amp"), String("lt"), String("gt")};

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
    return syntaxError(pos, "Unexpected end of file"), false;
  case '=':
    token.type = Token::equalsSignType;
    ++pos.pos;
    return true;
  case '"':
  case '\'':
    {
      char endChars[4] = "x\r\n";
      *endChars = *pos.pos;
      const char* end = String::findOneOf(pos.pos + 1, endChars);
      if(!end)
        return syntaxError(pos, "Unexpected end of file"), false;
      if(*end != *pos.pos)
        return syntaxError(pos, "New line in string"), false;
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
      const char* end = pos.pos;
      while(*end && *end != '/' && *end != '>' && *end != '=' && !String::isSpace(*end))
        ++end;
      if(end == pos.pos)
        return syntaxError(pos, "Expected name"), false;
      token.value = String(pos.pos, end - pos.pos);
      token.type = Token::nameType;
      pos.pos = end;
      return true;
    }
  }
}

void Xml::Private::skipSpace()
{
  for(char c;;)
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
      if(String::compare(pos.pos + 1, "!--", 3) == 0)
      {
        pos.pos += 4;
        for(;;)
        {
          const char* end = String::findOneOf(pos.pos, "-\n\r");
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
            if(String::compare(pos.pos + 1, "->", 2) == 0)
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
  const char* srcStart = str;
  const char* src = String::find(srcStart, '&');
  if(!src)
    return str;
  String result(str.length());
  char* destStart = result;
  usize startLen = src - srcStart;
  Memory::copy(destStart, srcStart, startLen * sizeof(char));
  char* dest = destStart + startLen;
  for(const char* srcEnd = srcStart + str.length(); src < srcEnd;)
    if(*src != '&')
      *(dest++) = *(src++);
    else
    {
      ++src;
      const char* sequenceEnd = String::find(src, ';');
      if(!sequenceEnd)
      {
        *(dest++) = '&';
        continue;
      }
      String str;
      str.attach(src, sequenceEnd - src);
      if (*src == '#')
      {
        uint unicodeValue;
        if(str.scanf("#%u", &unicodeValue) != 1)
        {
          *(dest++) = '&';
          continue;
        }
        String val = Unicode::toString(unicodeValue);
        Memory::copy(dest, (const char*)val, val.length() * sizeof(char));
        dest += val.length();
        src = sequenceEnd + 1;
        continue;
      }
      for(String* j = escapeStrings, * end = escapeStrings + sizeof(escapeStrings) / sizeof(*escapeStrings); j < end; ++j)
          if(str == *j)
          {
              *(dest++) = escapeChars[j - escapeStrings];
              src = sequenceEnd + 1;
              goto translated;
          }
          *(dest++) = '&';
          continue;
        translated:
          continue;
    }
  result.resize(dest - destStart);
  return result;
}

String Xml::Private::escapeString(const String& str)
{
  String result(str.length() + 200);
  char* destStart = result;
  char* dest = destStart;
  char c;
  for(const char* i = str, * end = i + str.length(); i < end; ++i)
  {
    c = *i;
    if((c & 0xc0) || (c & 0xe0) == 0) // c >= 64 || c < 32
    {
      *(dest++) = c;
      continue;
    }
    
    const char* escapeChar = String::find(escapeChars, c);
    if(escapeChar)
    {
      const String& escapeString = escapeStrings[escapeChar - escapeChars];
      result.resize(dest - destStart);
      result.reserve(result.length() + escapeString.length() + 1 + (end - i));
      destStart = result;
      dest = destStart + result.length();
      *(dest++) = '&';
      Memory::copy(dest, (const char*)escapeString, escapeString.length() * sizeof(char));
      dest += escapeString.length();
      *(dest++) = ';';
    }
    else
      *(dest++) = c;
  }
  result.resize(dest - destStart);
  return result;
}

bool Xml::Private::parse(const char* data, Element& element)
{
  pos.line = 1;
  pos.pos = pos.lineStart = data;

  skipSpace();
  while(*pos.pos == '<' && pos.pos[1] == '?')
  {
    Position startPos = pos;
    pos.pos += 2;
    for(;;)
    {
      const char* end = String::findOneOf(pos.pos, "\r\n?");
      if(!end)
        return this->syntaxError(startPos, "Unexpected end of file"), false;
      if(*end == '?' && end[1] == '>')
      {
        pos.pos = end + 2;
        break;
      }
      pos.pos = end + 1;
      skipSpace();
    }
    skipSpace();
  }
  if(!readToken())
    return false;
  if(token.type != Token::startTagBeginType)
    return syntaxError(token.pos, "Expected '<'"), false;
  return parseElement(element);
}

bool Xml::Private::parseElement(Element& element)
{
  element.line = token.pos.line;
  element.column = (int)(token.pos.pos - token.pos.lineStart) + 1;
  if(!readToken())
    return false;
  if(token.type != Token::nameType)
    return syntaxError(token.pos, "Expected tag name"), false;
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
        return syntaxError(token.pos, "Expected '='"), false;
      if(!readToken())
        return false;
      if(token.type != Token::stringType)
        return syntaxError(token.pos, "Expected string"), false;
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
    return syntaxError(token.pos, "Expected tag name"), false;
  if(token.value != element.type)
    return syntaxError(token.pos, String("Expected end tag of '") + element.type + ("'")), false;
  if(!readToken())
    return false;
  if(token.type != Token::tagEndType)
    return syntaxError(token.pos, "Expected '>'"), false;
  return true;
}

bool Xml::Private::parseText(String& text)
{
  const char* start = pos.pos;
  for(;;)
  {
    const char* end = String::findOneOf(pos.pos, "<\r\n");
    if(!end)
    {
      pos.pos = pos.pos + String::length(pos.pos);
      return syntaxError(pos, "Unexpected end of file"), false;
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

bool Xml::parse(const char* data, Element& element)
{
  Private parser;
  if(parser.parse(data, element))
    return true;
  Error::setErrorString(String::fromPrintf("Syntax error at line %d, column %d: %s", parser.errorLine, parser.errorColumn, (const char*)parser.errorString));
  return false;
}

bool Xml::parse(const String& data, Element& element)
{
  return parse((const char*)data, element);
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
  String result("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
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
    result.append("=\"");
    result.append(Xml::Private::escapeString(*i));
    result.append('"');
  }
  if(content.isEmpty())
    result.append("/>");
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
    result.append("</");
    result.append(type);
    result.append('>');
  }
  return result;
}
