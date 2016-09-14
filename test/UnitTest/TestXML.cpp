
#include <nstd/Debug.h>
#include <nstd/XML/XML.h>

void_t testXML()
{
  XML::Parser parser;
  XML::Element element;
  String input("<note><to x1=\"dasds\" xxx=\"dasdalal\">Tove</to><from>Jani</from><heading>Reminder</heading><body>Don't forget me this weekend!</body></note>");
  ASSERT(parser.parse(input, element));
  String output = element.toString();
  ASSERT(input == output);
}
