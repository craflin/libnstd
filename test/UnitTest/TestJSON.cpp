
#include <nstd/Debug.h>
#include <nstd/Document/JSON.h>

void_t testJSON()
{
  JSON::Parser parser;
  Variant data;
  String input(_T("{\n")
_T("  \"title\": \"Example Schema\",\n")
_T("  \"type\": \"object\",\n")
_T("  \"properties\": {\n")
_T("    \"firstName\": {\n")
_T("      \"type\": \"string\"\n")
_T("    },\n")
_T("    \"lastName\": {\n")
_T("      \"type\": \"string\"\n")
_T("    },\n")
_T("    \"age\": {\n")
_T("      \"description\": \"Age in years\",\n")
_T("      \"type\": \"integer\",\n")
_T("      \"minimum\": 0\n")
_T("    }\n")
_T("  },\n")
_T("  \"required\": [\"firstName\", \"lastName\"]\n")
_T("}\n"));
  ASSERT(parser.parse(input, data));
}
