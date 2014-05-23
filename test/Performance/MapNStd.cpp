
#include <nstd/Map.h>
#include <cstdlib>

void_t testMapNStd(int iterations)
{
  srand(100);
  Map<int, int> testMap;
  for (int i = 0; i < iterations; ++i)
  {
    testMap.insert(rand() % 1000, 0);
  }
  for (int i = 0; i < iterations; ++i)
  {
    testMap.insert(rand() % 1000, 0);
    testMap.remove(rand() % 1000);
  }
  for (int i = 0; i < iterations; ++i)
  {
    testMap.remove(rand() % 1000);
  }
}
