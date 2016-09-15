
#include <nstd/Debug.h>
#include <nstd/XML/XML.h>

void_t testXML()
{
  XML::Parser parser;
  XML::Element element;
  String input("<note>test&amp;test<to x1=\"dasds\" xxx=\"dasdalal\">Tove</to><from>Jani</from><heading>Reminder</heading><body>Don&apos;t forget me this weekend!</body></note>");
  ASSERT(parser.parse(input, element));
  ASSERT(element.type == "note");
  ASSERT(!element.content.isEmpty());
  ASSERT(element.content.front().toString() == "test&test");
  String output = element.toString();
  ASSERT(input == output);
}
