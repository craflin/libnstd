
#include <nstd/Document/Xml.hpp>
#include <nstd/Debug.hpp>

void testXml()
{
  Xml::Parser parser;
  Xml::Element element;
  String input(_T("<note>test&amp;test&#35;&#<to x1=\"dasds\" xxx=\"dasdalal\">Tove</to><from>Jani</from><heading>Reminder</heading><body>Don&apos;t forget me this weekend!</body></note>"));
  String expectedOutput(_T("<note>test&amp;test#&amp;#<to x1=\"dasds\" xxx=\"dasdalal\">Tove</to><from>Jani</from><heading>Reminder</heading><body>Don&apos;t forget me this weekend!</body></note>"));
  ASSERT(parser.parse(input, element));
  ASSERT(element.type == _T("note"));
  ASSERT(!element.content.isEmpty());
  ASSERT(element.content.front().toString() == _T("test&test#&#"));
  String output = element.toString();
  ASSERT(output == expectedOutput);
}

void testXmlSyntaxError()
{
  Xml::Parser parser;
  Xml::Element element;
  String input(_T("<a"));
  ASSERT(!parser.parse(input, element));
  ASSERT(parser.getErrorString() == _T("Unexpected end of file"));
}

int main(int argc, char* argv[])
{
  testXml();
  testXmlSyntaxError();
  return 0;
}
