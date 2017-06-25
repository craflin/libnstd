
#include <nstd/Debug.h>
#include <nstd/Future.h>
#include <nstd/String.h>

void testFuture()
{
  class FutureTest
  {
  public:
    bool called;

  public:
    FutureTest() : called(false), check(42) {}

    static usize staticTest0()
    {
      return 32;
    }

    static usize staticTest1(const String& str)
    {
      return str.length();
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

    usize memberTest4(const String& str1, const String& str2, const String& str3, const String& str4)
    {
      return check + str1.length() + str2.length() + str3.length() + str4.length();
    }

    static void staticVoidTest0()
    {
    }

    static void staticVoidTest1(int check)
    {
      ASSERT(check == 32);
    }

    static void staticVoidTest5(int check, const String& str1, const String& str2, const String& str3, const String& str4)
    {
      ASSERT(check == 32);
      ASSERT(str1 == "hallo1");
      ASSERT(str2 == "hallo2");
      ASSERT(str3 == "hallo3");
      ASSERT(str4 == "hallo4");
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

    void memberVoidTest4(int check, const String& str1, const String& str2, const String& str3)
    {
      called = true;
      ASSERT(this->check + check == 42 + 32);
      ASSERT(str1 == "hallo1");
      ASSERT(str2 == "hallo2");
      ASSERT(str3 == "hallo3");
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
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    future.start(&FutureTest::staticVoidTest1, 32);
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
  }

  {
    Future<void> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    future.start(&FutureTest::staticVoidTest5, 32, String("hallo1"), String("hallo2"), String("hallo3"), String("hallo4"));
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
  }

  {
    Future<void> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    FutureTest t;
    future.start(t, &FutureTest::memberVoidTest0);
    future.join();
    ASSERT(t.called);
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
  }

  {
    Future<void> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    FutureTest t;
    future.start(t, &FutureTest::memberVoidTest1, 32);
    future.join();
    ASSERT(t.called);
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
  }

  {
    Future<void> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    FutureTest t;
    future.start(t, &FutureTest::memberVoidTest4, 32, String("hallo1"), String("hallo2"), String("hallo3"));
    future.join();
    ASSERT(t.called);
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
  }

  {
    Future<usize> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    future.start(&FutureTest::staticTest0);
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
    ASSERT(future == 5);
  }

  {
    Future<usize> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    future.start(&FutureTest::staticTest1, String("hello"));
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
    ASSERT(future == 5);
  }

  {
    Future<usize> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    future.start(&FutureTest::staticTest5, String("hello"), String("hello"), String("hello"), String("hello"), String("hello"));
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
    ASSERT(future == 5);
  }

  {
    Future<usize> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    FutureTest t;
    future.start(t, &FutureTest::memberTest1, String("hello"));
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
    ASSERT(future == 42 + 5);
  }

  {
    Future<usize> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    FutureTest t;
    future.start(t, &FutureTest::memberTest4, String("hello1"), String("hello2"), String("hello3"), String("hello4"));
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
    ASSERT(future == 42 + 5);
  }
}
