
#include <nstd/HashMap.hpp>
#include <cstdlib>

void testHashMapNStd(int iterations)
{
  srand(100);
  HashMap<int, int> testMap(1000);
  for (int i = 0; i < iterations; ++i)
  {
    testMap.append(rand() % 1000, 0);
  }
  for (int i = 0; i < iterations; ++i)
  {
    testMap.append(rand() % 1000, 0);
    testMap.remove(rand() % 1000);
  }
  for (int i = 0; i < iterations; ++i)
  {
    testMap.remove(rand() % 1000);
  }
}
