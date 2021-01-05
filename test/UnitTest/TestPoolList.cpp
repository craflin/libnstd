
#include <nstd/PoolList.hpp>
#include <nstd/Debug.hpp>

struct TestObject
{
  int _a;

  TestObject() {}

  TestObject(int a) : _a(a) {}

private:
  TestObject(const TestObject&);
  TestObject& operator=(const TestObject&);
};

void testAppendAndRemove()
{
  PoolList<TestObject> pool;
  TestObject& obj = pool.append();
  ASSERT(pool.size() == 1);
  pool.remove(obj);
  ASSERT(pool.size() == 0);
}

void testClear()
{
  PoolList<TestObject> pool;
  pool.append();
  pool.append();
  pool.append();
  ASSERT(pool.size() == 3);
  pool.clear();
  ASSERT(pool.size() == 0);
}

void testSwap()
{
  PoolList<TestObject> pool;
  pool.append();
  pool.append();
  PoolList<TestObject> pool2;
  pool.swap(pool2);
  ASSERT(pool.size() == 0);
  ASSERT(pool2.size() == 2);
}

void testConstructorArgs()
{
  PoolList<TestObject> pool;
  pool.append(32);
  ASSERT(pool.begin()->_a == 32);
}

void testNoDefaultConstructor()
{
  struct TestObject2
  {
    int _a;

    TestObject2(int a) : _a(a) {}

  private:
    TestObject2() {}
    TestObject2(const TestObject2&) {}
    TestObject2& operator=(const TestObject2&) {}
  };

  PoolList<TestObject2> pool;
  pool.append(32);
  ASSERT(pool.begin()->_a == 32);
}

int main(int argc, char* argv[])
{
  testAppendAndRemove();
  testClear();
  testSwap();
  testConstructorArgs();
  testNoDefaultConstructor();
  return 0;
}
