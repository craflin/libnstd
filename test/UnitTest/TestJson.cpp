
#include <nstd/Document/Json.hpp>
#include <nstd/Debug.hpp>
#include <nstd/Unicode.hpp>

void testJson()
{
  Json::Parser parser;
  Variant data;
  String input("{\n"
"  \"title\": \"Example Schema\",\n"
"  \"surrogatePair\": \"aa\\uD834\\uDD1Ebb\",\n"
"  \"type\": \"object\",\n"
"  \"properties\": {\n"
"    \"firstName\": {\n"
"      \"type\": \"string\"\n"
"    },\n"
"    \"lastName\": {\n"
"      \"type\": \"string\"\n"
"    },\n"
"    \"age\": {\n"
"      \"description\": \"Age in years\",\n"
"      \"type\": \"integer\",\n"
"      \"minimum\": 0\n"
"    }\n"
"  },\n"
"  \"required\": [\"firstName\", \"lastName\"]\n"
"}\n");
  ASSERT(parser.parse(input, data));
  ASSERT(data.toMap().find("title")->toString() == "Example Schema");
  String surrogatePairCheck;
  surrogatePairCheck.append("aa");
  Unicode::append(0x1d11e, surrogatePairCheck);
  surrogatePairCheck.append("bb");
  ASSERT(data.toMap().find("surrogatePair")->toString() == surrogatePairCheck);
  ASSERT(data.toMap().find("type")->toString() ==  "object");
  ASSERT(data.toMap().find("properties")->toMap().find("firstName")->toMap().find("type")->toString() == "string");
  ASSERT(data.toMap().find("properties")->toMap().find("lastName")->toMap().find("type")->toString() == "string");
  ASSERT(data.toMap().find("properties")->toMap().find("age")->toMap().find("description")->toString() == "Age in years");
  ASSERT(data.toMap().find("properties")->toMap().find("age")->toMap().find("type")->toString() == "integer");
  ASSERT(data.toMap().find("properties")->toMap().find("age")->toMap().find("minimum")->getType() == Variant::intType);
  ASSERT(data.toMap().find("properties")->toMap().find("age")->toMap().find("minimum")->toInt() == 0);
  ASSERT(data.toMap().find("required")->toList().front().toString() == "firstName");
  ASSERT(data.toMap().find("required")->toList().back().toString() == "lastName");
}

void testStripComments()
{
  String input("{\n  \"test\": \"/*\",\n  /*\n*/\n  //\n  //a\n}\n");
  String stripped = Json::stripComments(input);
  String check("{\n  \"test\": \"/*\",\n  \n\n  \n  \n}\n");
  ASSERT(stripped == check);
}

int main(int argc, char* argv[])
{
    testJson();
    testStripComments();
    return 0;
}
