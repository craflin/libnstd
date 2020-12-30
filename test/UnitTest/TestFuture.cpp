
#include <nstd/Future.hpp>
#include <nstd/Debug.hpp>
#include <nstd/String.hpp>

void testFuture()
{
  class FutureTest
  {
  public:
    bool called;

  public:
    FutureTest() : check(42) {}

    static void staticVoidTest0()
    {
    }

    static void staticVoidTest1(int check)
    {
      ASSERT(check == 32);
    }

    static void staticVoidTest2(int check, const String& str1)
    {
      ASSERT(check == 32);
      ASSERT(str1 == _T("hallo1"));
    }

    static void staticVoidTest3(int check, const String& str1, const String& str2)
    {
      ASSERT(check == 32);
      ASSERT(str1 == _T("hallo1"));
      ASSERT(str2 == _T("hallo2"));
    }

    static void staticVoidTest4(int check, const String& str1, const String& str2, const String& str3)
    {
      ASSERT(check == 32);
      ASSERT(str1 == _T("hallo1"));
      ASSERT(str2 == _T("hallo2"));
      ASSERT(str3 == _T("hallo3"));
    }

    static void staticVoidTest5(int check, const String& str1, const String& str2, const String& str3, const String& str4)
    {
      ASSERT(check == 32);
      ASSERT(str1 == _T("hallo1"));
      ASSERT(str2 == _T("hallo2"));
      ASSERT(str3 == _T("hallo3"));
      ASSERT(str4 == _T("hallo4"));
    }

    void memberVoidTest0()
    {
      called = true;
    }

    void memberVoidTest1(int check)
    {
      called = true;
      ASSERT(this->check + check == 42 + 32);
    }

    void memberVoidTest2(int check, const String& str1)
    {
      called = true;
      ASSERT(this->check + check == 42 + 32);
      ASSERT(str1 == _T("hallo1"));
    }

    void memberVoidTest3(int check, const String& str1, const String& str2)
    {
      called = true;
      ASSERT(this->check + check == 42 + 32);
      ASSERT(str1 == _T("hallo1"));
      ASSERT(str2 == _T("hallo2"));
    }

    void memberVoidTest4(int check, const String& str1, const String& str2, const String& str3)
    {
      called = true;
      ASSERT(this->check + check == 42 + 32);
      ASSERT(str1 == _T("hallo1"));
      ASSERT(str2 == _T("hallo2"));
      ASSERT(str3 == _T("hallo3"));
    }

    static usize staticTest0()
    {
      return 32;
    }

    static usize staticTest1(const String& str)
    {
      return str.length();
    }

    static usize staticTest2(const String& str1, const String& str2)
    {
      return str1.length() + str2.length();
    }

    static usize staticTest3(const String& str1, const String& str2, const String& str3)
    {
      return str1.length() + str2.length() + str3.length();
    }

    static usize staticTest4(const String& str1, const String& str2, const String& str3, const String& str4)
    {
      return str1.length() + str2.length() + str3.length() + str4.length();
    }

    static usize staticTest5(const String& str1, const String& str2, const String& str3, const String& str4, const String& str5)
    {
      return str1.length() + str2.length() + str3.length() + str4.length() + str5.length();
    }

    usize memberTest0()
    {
      return check;
    }

    usize memberTest1(const String& str)
    {
      return check + str.length();
    }

    usize memberTest2(const String& str1, const String& str2)
    {
      return check + str1.length() + str2.length();
    }

    usize memberTest3(const String& str1, const String& str2, const String& str3)
    {
      return check + str1.length() + str2.length() + str3.length();
    }

    usize memberTest4(const String& str1, const String& str2, const String& str3, const String& str4)
    {
      return check + str1.length() + str2.length() + str3.length() + str4.length();
    }

  private:
    int check;
  };

  {
    Future<void> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    future.start(&FutureTest::staticVoidTest0);
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
  }

  {
    Future<void> future;
    future.start(&FutureTest::staticVoidTest1, 32);
    future.join();
  }

  {
    Future<void> future;
    future.start(&FutureTest::staticVoidTest2, 32, String(_T("hallo1")));
    future.join();
  }

  {
    Future<void> future;
    future.start(&FutureTest::staticVoidTest3, 32, String(_T("hallo1")), String(_T("hallo2")));
    future.join();
  }

  {
    Future<void> future;
    future.start(&FutureTest::staticVoidTest4, 32, String(_T("hallo1")), String(_T("hallo2")), String(_T("hallo3")));
    future.join();
  }

  {
    Future<void> future;
    future.start(&FutureTest::staticVoidTest5, 32, String(_T("hallo1")), String(_T("hallo2")), String(_T("hallo3")), String(_T("hallo4")));
    future.join();
  }

  {
    Future<void> future;
    FutureTest t;
    future.start(t, &FutureTest::memberVoidTest0);
    future.join();
    ASSERT(t.called);
  }

  {
    Future<void> future;
    FutureTest t;
    future.start(t, &FutureTest::memberVoidTest1, 32);
    future.join();
    ASSERT(t.called);
  }

  {
    Future<void> future;
    FutureTest t;
    future.start(t, &FutureTest::memberVoidTest2, 32, String(_T("hallo1")));
    future.join();
    ASSERT(t.called);
  }

  {
    Future<void> future;
    FutureTest t;
    future.start(t, &FutureTest::memberVoidTest3, 32, String(_T("hallo1")), String(_T("hallo2")));
    future.join();
    ASSERT(t.called);
  }

  {
    Future<void> future;
    FutureTest t;
    future.start(t, &FutureTest::memberVoidTest4, 32, String(_T("hallo1")), String(_T("hallo2")), String(_T("hallo3")));
    future.join();
    ASSERT(t.called);
  }

  {
    Future<usize> future;
    future.start(&FutureTest::staticTest0);
    future.join();
    ASSERT(future == 32);
  }

  {
    Future<usize> future;
    future.start(&FutureTest::staticTest1, String(_T("hello")));
    future.join();
    ASSERT(future == 5);
  }

  {
    Future<usize> future;
    future.start(&FutureTest::staticTest2, String(_T("hello")), String(_T("hello")));
    future.join();
    ASSERT(future == 10);
  }

  {
    Future<usize> future;
    future.start(&FutureTest::staticTest3, String(_T("hello")), String(_T("hello")), String(_T("hello")));
    future.join();
    ASSERT(future == 15);
  }

  {
    Future<usize> future;
    future.start(&FutureTest::staticTest4, String(_T("hello")), String(_T("hello")), String(_T("hello")), String(_T("hello")));
    future.join();
    ASSERT(future == 20);
  }

  {
    Future<usize> future;
    future.start(&FutureTest::staticTest5, String(_T("hello")), String(_T("hello")), String(_T("hello")), String(_T("hello")), String(_T("hello")));
    future.join();
    ASSERT(future == 25);
  }

  {
    Future<usize> future;
    FutureTest t;
    future.start(t, &FutureTest::memberTest1, String(_T("hello")));
    future.join();
    ASSERT(future == 42 + 5);
  }

  {
    Future<usize> future;
    FutureTest t;
    future.start(t, &FutureTest::memberTest2, String(_T("hello1")), String(_T("hello2")));
    future.join();
    ASSERT(future == 42 + 6 * 2);
  }

  {
    Future<usize> future;
    FutureTest t;
    future.start(t, &FutureTest::memberTest3, String(_T("hello1")), String(_T("hello2")), String(_T("hello3")));
    future.join();
    ASSERT(future == 42 + 6 * 3);
  }

  {
    Future<usize> future;
    FutureTest t;
    future.start(t, &FutureTest::memberTest4, String(_T("hello1")), String(_T("hello2")), String(_T("hello3")), String(_T("hello4")));
    future.join();
    ASSERT(future == 42 + 6 * 4);
  }
}

int main(int argc, char* argv[])
{
    testFuture();
    return 0;
}
