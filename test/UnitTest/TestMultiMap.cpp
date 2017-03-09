
#include <nstd/Debug.h>
#include <nstd/MultiMap.h>

void testMultiMap()
{
  // test insert
  {
    MultiMap<int, bool> map;
    map.insert(1, false);
    map.insert(2, false);
    map.insert(2, false);
    map.insert(3, false);
    ASSERT(map.count(1) == 1);
    ASSERT(map.count(2) == 2);
    ASSERT(map.count(3) == 1);
    ASSERT(map.count(4) == 0);
  }

  // test clear
  {
    MultiMap<int, bool> map;
    map.insert(1, false);
    ASSERT(map.size() == 1);
    map.clear();
    ASSERT(map.size() == 0);
    map.insert(1, false);
    ASSERT(map.size() == 1);
    ASSERT(++map.begin() == map.end());
  }
}

