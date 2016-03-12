
#include <nstd/Debug.h>
#include <nstd/MultiMap.h>

void_t testMultiMap()
{
  MultiMap<int_t, bool_t> map;
  map.insert(1, false);
  map.insert(2, false);
  map.insert(2, false);
  map.insert(3, false);
  ASSERT(map.count(1) == 1);
  ASSERT(map.count(2) == 2);
  ASSERT(map.count(3) == 1);
  ASSERT(map.count(4) == 0);
}

