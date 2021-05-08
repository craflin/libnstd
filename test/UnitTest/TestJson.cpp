
#include <nstd/Document/Json.hpp>
#include <nstd/Debug.hpp>
#include <nstd/Unicode.hpp>

void testJson()
{
  Json::Parser parser;
  Variant data;
  String input(_T("{\n")
_T("  \"title\": \"Example Schema\",\n")
_T("  \"surrogatePair\": \"aa\\uD834\\uDD1Ebb\",\n")
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
  ASSERT(data.toMap().find(_T("title"))->toString() == _T("Example Schema"));
  String surrogatePairCheck;
  surrogatePairCheck.append(_T("aa"));
  Unicode::append(0x1d11e, surrogatePairCheck);
  surrogatePairCheck.append(_T("bb"));
  ASSERT(data.toMap().find(_T("surrogatePair"))->toString() == surrogatePairCheck);
  ASSERT(data.toMap().find(_T("type"))->toString() ==  _T("object"));
  ASSERT(data.toMap().find(_T("properties"))->toMap().find(_T("firstName"))->toMap().find(_T("type"))->toString() == _T("string"));
  ASSERT(data.toMap().find(_T("properties"))->toMap().find(_T("lastName"))->toMap().find(_T("type"))->toString() == _T("string"));
  ASSERT(data.toMap().find(_T("properties"))->toMap().find(_T("age"))->toMap().find(_T("description"))->toString() == _T("Age in years"));
  ASSERT(data.toMap().find(_T("properties"))->toMap().find(_T("age"))->toMap().find(_T("type"))->toString() == _T("integer"));
  ASSERT(data.toMap().find(_T("properties"))->toMap().find(_T("age"))->toMap().find(_T("minimum"))->getType() == Variant::intType);
  ASSERT(data.toMap().find(_T("properties"))->toMap().find(_T("age"))->toMap().find(_T("minimum"))->toInt() == 0);
  ASSERT(data.toMap().find(_T("required"))->toList().front().toString() == _T("firstName"));
  ASSERT(data.toMap().find(_T("required"))->toList().back().toString() == _T("lastName"));
}

void testStripComments()
{
  String input(_T("{\n  \"test\": \"/*\",\n  /*\n*/\n  //\n  //a\n}\n"));
  String stripped = Json::stripComments(input);
  String check(_T("{\n  \"test\": \"/*\",\n  \n\n  \n  \n}\n"));
  ASSERT(stripped == check);
}

int main(int argc, char* argv[])
{
    testJson();
    testStripComments();
    return 0;
}
