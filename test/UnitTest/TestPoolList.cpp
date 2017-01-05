
#include <nstd/Debug.h>
#include <nstd/PoolList.h>

void testPoolList()
{
  struct TestObject
  {
    int a;
  };

  // test append and remove
  PoolList<TestObject> pool;
  TestObject& obj = pool.append();
  ASSERT(pool.size() == 1);
  pool.remove(obj);
  ASSERT(pool.size() == 0);

  // test clear
  pool.clear();
  pool.append();
  pool.append();
  pool.append();
  ASSERT(pool.size() == 3);
  pool.clear();
  ASSERT(pool.size() == 0);

  // test swap
  pool.clear();
  pool.append();
  pool.append();
  PoolList<TestObject> pool2;
  pool.swap(pool2);
  ASSERT(pool.size() == 0);
  ASSERT(pool2.size() == 2);
}
