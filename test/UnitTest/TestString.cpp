
#include <nstd/Debug.h>
#include <nstd/String.h>

#include <cctype>

void_t testString()
{
  // test constructors
  String empty;
  ASSERT(empty.isEmpty());
  String hello(_T("hello"));
  ASSERT(!hello.isEmpty());
  String copyOfHello(hello);
  ASSERT(!copyOfHello.isEmpty());
  String copyOfCopyOfHello(copyOfHello);
  ASSERT(!copyOfCopyOfHello.isEmpty());

  // test assignment operator
  {
    String x(copyOfCopyOfHello);
    x = copyOfHello;
    x = String(_T("omg"));
    String y(_T("omg"));
    y = y;
  }
  

  // test self assign
  String blupp;
  blupp.printf(_T("%d"), 123);
  blupp = blupp;
  ASSERT(blupp == _T("123"));

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

  // test printf
  empty.printf(_T("%s %s"), (const tchar_t*)hello, _T("world"));
  ASSERT(empty == _T("hello world"));
  ASSERT(empty != _T("hello worl2"));
  ASSERT(empty != _T("hello worl2a"));

  // test toUpperCase, toLowerCase, isSpace
  for (int i = 0; i < 0x100; ++i)
  {
#ifdef _UNICODE
    ASSERT(String::toUpperCase((wchar_t)i) == (wchar_t)towupper(i));
    ASSERT(String::toLowerCase((wchar_t)i) == (wchar_t)towlower(i));
    ASSERT(String::isSpace((wchar_t)i) == !!iswspace(i));
#else
    ASSERT(String::toUpperCase((char_t)i) == (char_t)toupper((uchar_t&)i));
    ASSERT(String::toLowerCase((char_t)i) == (char_t)tolower((uchar_t&)i));
    ASSERT(String::isSpace((char_t)i) == !!isspace((uchar_t&)i));
#endif
  }

  // test static length
  ASSERT(String::length(_T("")) == 0);
  ASSERT(String::length(_T("123")) == 3);

  // test find methods
  String test(_T("this is the find test test string"));
  ASSERT(test.find(_T('z')) == 0);
  ASSERT(test.find(_T("zz")) == 0);
  ASSERT(String::compare(test.find(_T('i')), _T("is is the find test test string")) == 0);
  ASSERT(String::compare(test.findLast(_T('i')), _T("ing")) == 0);
  ASSERT(String::compare(test.find(_T("is")), _T("is is the find test test string")) == 0);
  ASSERT(String::compare(test.findLast(_T("is")), _T("is the find test test string")) == 0);
  ASSERT(String::compare(test.findOneOf(_T("ex")), _T("e find test test string")) == 0);
  ASSERT(String::compare(test.findOneOf(_T("xe")), _T("e find test test string")) == 0);
  ASSERT(String::compare(test.findLastOf(_T("ex")), _T("est string")) == 0);
  ASSERT(String::compare(test.findLastOf(_T("xe")), _T("est string")) == 0);

  // test prepend, append
  String b(_T(" b "));
  String a(_T("aa"));
  String c(_T("cc"));
  ASSERT(a + b + c == _T("aa b cc"));
  ASSERT(String().append(a).append(b).append(c) == _T("aa b cc"));
  ASSERT(String().append(b).prepend(a).append(c) == _T("aa b cc"));

  // test if lazy copying
  struct LazyCopyTest
  {
    static String test1()
    {
      return String(_T("test"));
    }
  };
  String aa = LazyCopyTest::test1(); // so, this is equal to "String aa(_T("test"))"?
  const tchar_t* caa = aa;
  ASSERT(caa != (tchar_t*)aa);

  // test external buffer attaching
  tchar_t buf[100];
  for(tchar_t* i = buf; i < buf + 8; ++i)
    *i = _T('a');
  String bufWrapper;
  bufWrapper.attach(buf, 4);
  ASSERT(bufWrapper == String(buf, 4));
  ASSERT(bufWrapper == String(_T("aaaa")));

  // test detach
  {
    tchar_t buf[100];
    for(size_t i = 0; i < 8; ++i)
      buf[i] = _T('a');
    String bufWrapper;
    buf[8] = _T('\0');
    bufWrapper.attach(buf, 8);
    bufWrapper.detach();
    buf[2] = _T('b');
    ASSERT(bufWrapper == _T("aaaaaaaa"));
  }
}
