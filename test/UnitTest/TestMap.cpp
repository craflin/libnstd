
#include <nstd/Debug.h>
#include <nstd/Map.h>
#include <nstd/String.h>
#include <nstd/Math.h>

void testMap()
{
  // test insert
  {
    Map<String, int32> map;
    ASSERT(map.isEmpty());
    ASSERT(map.size() == 0);
    ASSERT(map.begin() == map.end());
    map.insert(_T("000"), 0);
    ASSERT(map.begin() != map.end());
    Map<String, int32>::Iterator begin = map.begin();
    ASSERT(++begin == map.end());
    for(int i = 1; i < 90; ++i)
    {
      String item;
      item.printf(_T("%03d"), i);
      map.insert(item, i);
    }
    for(int i = 90; i < 100; ++i)
    {
      String item;
      item.printf(_T("%03d"), i);
      map.insert(map.end(), item, i);
    }
    ASSERT(!map.isEmpty());
    ASSERT(map.size() == 100);
    int i = 0;
    for(Map<String, int32>::Iterator j = map.begin(), end = map.end(); j != end; ++j, ++i)
    {
      String item;
      item.printf(_T("%03d"), i);
      ASSERT(j.key() == item);
      ASSERT(*j == i);
    }
    for(int i = 0; i < 100; i += 10)
    {
      String item;
      item.printf(_T("%03d"), i);
      map.insert(item, i);
    }
    for(int i = 4; i < 20; i += 10)
    {
      String item2;
      item2.printf(_T("%03d"), 99 - i);
      Map<String, int32>::Iterator testInsertPos = map.find(item2);
      String item;
      item.printf(_T("%03d"), i);
      map.insert(testInsertPos, item, i);
    }
    for(int i = 3; i < 100; i += 10)
    {
      String item2;
      item2.printf(_T("%03d"), i);
      Map<String, int32>::Iterator testInsertPos = map.find(item2);
      String item;
      item.printf(_T("%03d"), i);
      map.insert(testInsertPos, item, i);
    }
    for(int i = 6; i < 100; i += 10)
    {
      String item2;
      item2.printf(_T("%03d"), i - 5);
      Map<String, int32>::Iterator testInsertPos = map.find(item2);
      String item;
      item.printf(_T("%03d"), i);
      map.insert(testInsertPos, item, i);
    }
    String lastKey = map.begin().key();
    int lastValue = *map.begin();
    for(Map<String, int32>::Iterator k = ++Map<String, int32>::Iterator(map.begin()), end = map.end(); k != end; ++k)
    {
      ASSERT(k.key() > lastKey);
      ASSERT(*k > lastValue);
      lastKey = k.key();
      lastValue = *k; 
    }
    map.remove(_T("042"));
  }

  // test random insert and remove
  {
    Map<int32, int32> map;
    for(int i = 0; i < 10000; ++i)
    {
      Map<int32, int32>::Iterator testInsertPos = map.find(Math::random() % 100);
      map.insert(testInsertPos, i % 100, 123);
    }
    int lastKey = map.begin().key();
    for(Map<int32, int32>::Iterator k = ++Map<int32, int32>::Iterator(map.begin()), end = map.end(); k != end; ++k)
    {
      ASSERT(k.key() > lastKey);
      lastKey = k.key();
    }
    int item;
    for(int i = 0; i < 5000; ++i)
    {
      Map<int32, int32>::Iterator testRmPos = map.find(Math::random() % 100);
      if (testRmPos != map.end())
        map.remove(testRmPos);
      item = Math::random() % 100;
      map.insert(item, 123);
    }
    lastKey = map.begin().key();
    usize count = 1;
    for(Map<int32, int32>::Iterator k = ++Map<int32, int32>::Iterator(map.begin()), end = map.end(); k != end; ++k)
    {
      ASSERT(k.key() > lastKey);
      lastKey = k.key();
      ++count;
    }
    ASSERT(count == map.size());
    for(Map<int32, int32>::Iterator i = map.begin(), end = map.end(); i != end; ++i)
    {
      ASSERT(map.find(i.key()) != map.end());
    }
  }

  // test clear
  {
    Map<int, bool> map;
    map.insert(1, false);
    ASSERT(map.size() == 1);
    map.clear();
    ASSERT(map.size() == 0);
    map.insert(1, false);
    ASSERT(map.size() == 1);
    ASSERT(++map.begin() == map.end());
  }
}
