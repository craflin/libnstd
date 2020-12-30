
#include <nstd/PoolMap.hpp>
#include <nstd/Debug.hpp>
#include <nstd/String.hpp>

struct TestObject
{
  int a;

  TestObject() {}

private:
  TestObject(const TestObject&);
  TestObject& operator=(const TestObject&);
};

void testPoolMap()
{
  // test append and remove
  PoolMap<String, TestObject> pool;
  TestObject& obj = pool.append(_T("object1"));
  ASSERT(pool.size() == 1);
  pool.remove(obj);
  ASSERT(pool.size() == 0);

  // test clear
  pool.clear();
  pool.append(_T("object1"));
  pool.append(_T("object2"));
  pool.append(_T("object3"));
  ASSERT(pool.size() == 3);
  pool.clear();
  ASSERT(pool.size() == 0);

  // test swap
  pool.clear();
  pool.append(_T("object1"));
  pool.append(_T("object2"));
  PoolMap<String, TestObject> pool2;
  pool.swap(pool2);
  ASSERT(pool.size() == 0);
  ASSERT(pool2.size() == 2);
}

int main(int argc, char* argv[])
{
  testPoolMap();
  return 0;
}
