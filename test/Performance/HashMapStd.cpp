
#include <unordered_map>
#include <cstdlib>

void testHashMapStd(int iterations)
{
  srand(100);
  std::unordered_map<int, int> testMap(1000);
  for (int i = 0; i < iterations; ++i)
  {
    testMap.insert(std::make_pair(rand() % 1000, 0));
  }
  for (int i = 0; i < iterations; ++i)
  {
    testMap.insert(std::make_pair(rand() % 1000, 0));
    testMap.erase(rand() % 1000);
  }
  for (int i = 0; i < iterations; ++i)
  {
    testMap.erase(rand() % 1000);
  }
}

