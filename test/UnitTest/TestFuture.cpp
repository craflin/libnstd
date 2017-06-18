
#include <nstd/Debug.h>
#include <nstd/Future.h>
#include <nstd/String.h>

template<typename T> class How;

template<> class How<void>
{
};

template<typename T> class How : public How<void>
{
public:
  void dasd() {}
};


void testFuture()
{
  //How<int> lol;
  //void* x = &lol.dasd();


  class FutureTest
  {
  public:
    FutureTest() : check(42) {}

    usize test1(const String& str)
    {
      return str.length() + check;
    }

    static usize test2(const String& str)
    {
      return str.length();
    }

    static void testVoid(int* const& check)
    {
      *check = 32;
    }

  private:
    int check;
  };

  {
    Future<usize> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    future.start(&FutureTest::test2, String("hello"));
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
    ASSERT(future == 5);
  }

{
    Future<void> future;
    ASSERT(!future.isAborting());
    ASSERT(!future.isFinished());
    ASSERT(!future.isAborted());
    int x = 23;
    future.start(&FutureTest::testVoid, &x);
    future.join();
    ASSERT(!future.isAborting());
    ASSERT(future.isFinished());
    ASSERT(!future.isAborted());
    ASSERT(x == 32);
  }
}
