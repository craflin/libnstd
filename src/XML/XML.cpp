
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
    const char_t* pos;
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
  const char_t* start;
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

  static String unescapeString(const String& str);
};

class XML::Element::Private
{
public:
  static String escapeString(const String& str);
};

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
      char_t endChars[4] = "x\r\n";
      *endChars = *pos.pos;
      const char_t* end = String::findOneOf(pos.pos + 1, endChars);
      if(!end)
        return syntaxError(pos, "Unexpected end of file"), false;
      if(*end != *pos.pos)
        return syntaxError(pos, "New line in string"), false;
      token.pos = pos;
      token.value = unescapeString(String(pos.pos + 1, end - pos.pos - 1));
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
      const char_t* end = pos.pos;
      while(*end && *end != '/' && *end != '>' && *end != '=' && !String::isSpace(*end))
        ++end;
      if(end == pos.pos)
        return syntaxError(pos, "Expected name"), false;
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
  for(char_t c;;)
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
      if(String::compare(pos.pos + 1, "!--") == 0)
      {
        const char_t* end = pos.pos + 4;
        for(;;)
        {
          int_t line = pos.line;
          const char_t* newEnd = String::findOneOf(end, "-\n\r");
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
            if(String::compare(end + 1, "->") == 0)
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
  for(const char_t* p = pos.pos; p > start;)
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

String XML::Parser::Private::unescapeString(const String& str)
{
  // todo
  return str;
}

String XML::Element::Private::escapeString(const String& str)
{
  // todo
  return str;
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
      const char* end = String::findOneOf(pos.pos, "\r\n?");
      if(!end)
        return this->syntaxError(pos, "Unexpected end of file"), false;
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
    return syntaxError(token.pos, "Expected '<'"), false;
  return parseElement(element);
}

bool_t XML::Parser::Private::parseElement(Element& element)
{
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
    return syntaxError(token.pos, String("Expected end tag of '") + element.type + "'"), false;
  if(!readToken())
    return false;
  if(token.type != Token::tagEndType)
    return syntaxError(token.pos, "Expected '>'"), false;
  return true;
}

bool_t XML::Parser::Private::parseText(String& text)
{
  const char_t* start = pos.pos;
  int_t line = pos.line;
  const char_t* end = pos.pos;
  for(;;)
  {
    const char_t* newEnd = String::findOneOf(end, "<\r\n");
    if(!newEnd)
    {
      pos.pos = end + String::length(end);
      return syntaxError(pos, "Unexpected end of file"), false;
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
      text = String(start, end - start);
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
  Error::setErrorString(String::fromPrintf("Syntax error at line %d, column %d: %s", parser.getErrorLine(), parser.getErrorColumn(), (const char_t*)parser.getErrorString()));
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
  String result("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
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
    result.append("=\"");
    result.append(Private::escapeString(*i));
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
        result.append(variant.toString());
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
