
#include <nstd/String.hpp>
#include <nstd/Debug.hpp>
#include <nstd/List.hpp>

#include <cctype>

void testString()
{
  // test constructors
  String empty;
  ASSERT(empty.isEmpty());
  String hello("hello");
  ASSERT(!hello.isEmpty());
  String copyOfHello(hello);
  ASSERT(!copyOfHello.isEmpty());
  String copyOfCopyOfHello(copyOfHello);
  ASSERT(!copyOfCopyOfHello.isEmpty());

  // test assignment operator
  {
    String x(copyOfCopyOfHello);
    x = copyOfHello;
    x = String("omg");
    String y("omg");
    y = y;
  }

  // test comparison
  ASSERT(String("123").compare("123") == 0);
  ASSERT(String("123").compare("xxx") != 0);
  ASSERT(String("1234").compare("123", 3) == 0);
  ASSERT(String("1234").compare("xxx", 3) != 0);
  ASSERT(String("123").compare("1234", 3) == 0);
  ASSERT(String("xxx").compare("1234", 3) != 0);

  // test ignore case comparison
  ASSERT(String("abc").compareIgnoreCase("ABC") == 0);
  ASSERT(String("abc").compareIgnoreCase("xxx") != 0);
  ASSERT(String("abcd").compareIgnoreCase("ABC", 3) == 0);
  ASSERT(String("abcd").compareIgnoreCase("xxx", 3) != 0);
  ASSERT(String("abc").compareIgnoreCase("ABCD", 3) == 0);
  ASSERT(String("xxx").compareIgnoreCase("ABCD", 3) != 0);

  // test self assign
  String blupp;
  blupp.printf("%d", 123);
  blupp = blupp;
  ASSERT(blupp == "123");

  // test compare operators
  ASSERT(hello == copyOfCopyOfHello);
  ASSERT(copyOfHello == copyOfCopyOfHello);
  ASSERT(hello == copyOfCopyOfHello);
  ASSERT(hello != empty);
  ASSERT(copyOfHello != empty);
  ASSERT(copyOfCopyOfHello != empty);
  ASSERT(!(hello == empty));
  ASSERT(!(copyOfHello == empty));
  ASSERT(!(copyOfCopyOfHello == empty));

  // test clear
  copyOfHello.clear();
  ASSERT(copyOfHello.isEmpty());

  // test toUpperCase, toLowerCase, isSpace
  for (int i = 0; i < 0x100; ++i)
  {
    ASSERT(String::toUpperCase((char)i) == (char)toupper((uchar&)i));
    ASSERT(String::toLowerCase((char)i) == (char)tolower((uchar&)i));
    ASSERT(String::isSpace((char)i) == !!isspace((uchar&)i));
  }

  // test static length
  ASSERT(String::length("") == 0);
  ASSERT(String::length("123") == 3);

  // test find methods
  String test("this is the find test test string");
  ASSERT(test.find('z') == 0);
  ASSERT(test.find("zz") == 0);
  ASSERT(String::compare(test.find('i'), "is is the find test test string") == 0);
  ASSERT(String::compare(test.findLast('i'), "ing") == 0);
  ASSERT(String::compare(test.find("is"), "is is the find test test string") == 0);
  ASSERT(String::compare(test.findLast("is"), "is the find test test string") == 0);
  ASSERT(String::compare(test.findOneOf("ex"), "e find test test string") == 0);
  ASSERT(String::compare(test.findOneOf("xe"), "e find test test string") == 0);
  ASSERT(String::compare(test.findLastOf("ex"), "est string") == 0);
  ASSERT(String::compare(test.findLastOf("xe"), "est string") == 0);

  // test prepend, append
  String b(" b ");
  String a("aa");
  String c("cc");
  ASSERT(a + b + c == "aa b cc");
  ASSERT(String().append(a).append(b).append(c) == "aa b cc");
  ASSERT(String().append(b).prepend(a).append(c) == "aa b cc");

  // test if lazy copying
  struct LazyCopyTest
  {
    static String test1()
    {
      return String("test");
    }
  };
  String aa = LazyCopyTest::test1(); // so, this is equal to "String aa("test")"?
  const char* caa = aa;
  ASSERT(caa != (char*)aa);

  // test external buffer attaching
  char buf[100];
  for(char* i = buf; i < buf + 8; ++i)
    *i = 'a';
  String bufWrapper;
  bufWrapper.attach(buf, 4);
  ASSERT(bufWrapper == String(buf, 4));
  ASSERT(bufWrapper == String("aaaa"));

  // test detach
  {
    char buf[100];
    for(usize i = 0; i < 8; ++i)
      buf[i] = 'a';
    String bufWrapper;
    buf[8] = '\0';
    bufWrapper.attach(buf, 8);
    bufWrapper.detach();
    buf[2] = 'b';
    ASSERT(bufWrapper == "aaaaaaaa");
  }

  // test trim
  ASSERT(String().trim() == String());
  ASSERT(String("\t \n\t \n").trim() == String());
  ASSERT(String("\t \nx\t \n").trim() == "x");
  ASSERT(String("x\t \n").trim() == "x");
  ASSERT(String("\t \nx").trim() == "x");
  ASSERT(String("x").trim() == "x");

  // test toBool
  ASSERT(!String().toBool());
  ASSERT(!String("").toBool());
  ASSERT(!String("0").toBool());
  ASSERT(!String("false").toBool());
  ASSERT(!String("False").toBool());
  ASSERT(!String("falSe").toBool());
  ASSERT(!String("0.0").toBool());
  ASSERT(!String(".0").toBool());
  ASSERT(!String("0.").toBool());
  ASSERT(!String(".00").toBool());
  ASSERT(!String("00.").toBool());
  ASSERT(!String("00.00").toBool());
  ASSERT(!String("0.00").toBool());
  ASSERT(!String("00.0").toBool());
  ASSERT(String(".").toBool());
  ASSERT(String("dasdas").toBool());
  ASSERT(String("true").toBool());

  // test join
  {
    String str;
    ASSERT(str.join(List<String>(), '.') == String());
  }
}

void testPrintf()
{
  String hello("hello");
  String empty;
  empty.printf("%s %s", (const char*)hello, "world");
  ASSERT(empty == "hello world");
  ASSERT(empty != "hello worl2");
  ASSERT(empty != "hello worl2a");
  char buf[512 + 1];
  Memory::fill(buf, 'b', 3);
  Memory::fill(buf + 3, 'a', 512 - 3);
  buf[512] = '\0';
  empty.printf("%s%s", "bbb", buf + 3);
  ASSERT(empty == buf);
}

void testBase64()
{
    ASSERT(String::fromBase64("bGlnaHQgd29yay4=") == "light work.");
    ASSERT(String::fromBase64("bGlnaHQgd29yaw==") == "light work");
    ASSERT(String::fromBase64("bGlnaHQgd29y") == "light wor");
    ASSERT(String::fromBase64("bGlnaHQgd28=") == "light wo");
    ASSERT(String::fromBase64("bGlnaHQgdw==") == "light w");
}

int main(int argc, char* argv[])
{
  testString();
  testPrintf();
  testBase64();
  return 0;
}
