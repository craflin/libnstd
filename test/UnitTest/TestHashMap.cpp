
#include <nstd/HashMap.hpp>
#include <nstd/Debug.hpp>
#include <nstd/String.hpp>

void testHashMap()
{
  {
    // test append
    HashMap<int, int> myMap;
    HashMap<int, int> emptyMap;
    myMap.swap(emptyMap);
    ASSERT(myMap.isEmpty());
    myMap.append(123, 123);
    ASSERT(myMap.contains(123));
    myMap.append(123, 125);
    ASSERT(myMap.size() == 1);
    ASSERT(*myMap.find(123) == 125);

    // test clear
    myMap.clear();
    ASSERT(myMap.size() == 0);
    ASSERT(myMap.isEmpty());
  }

  {
    // test append
    HashMap<String, int> myMap;
    ASSERT(myMap.isEmpty());
    myMap.append("string1", 120);
    myMap.append("string1", 121);
    ASSERT(myMap.size() == 1);
    myMap.append("string2", 122);
    myMap.append("string3", 123);
    ASSERT(myMap.size() == 3);

    // test find
    ASSERT(*myMap.find("string1") == 121);
    ASSERT(*myMap.find("string2") == 122);
    ASSERT(*myMap.find("string3") == 123);
    ASSERT(myMap.find("dsadasa") == myMap.end());

    // test list copy constructor
    HashMap<String, int> myMap2(myMap);
    ASSERT(myMap2.size() == 3);
    ASSERT(*myMap2.begin() == 121);
    ASSERT(myMap2.begin().key() == "string1");
    ASSERT(*(++HashMap<String, int>::Iterator(myMap2.begin())) == 122);
    ASSERT(myMap2.back() == 123);

    // test list copy operator
    HashMap<String, int> myMap3;
    myMap3 = myMap;
    ASSERT(myMap3.size() == 3);
    ASSERT(*myMap3.begin() == 121);
    ASSERT(myMap3.begin().key() == "string1");
    ASSERT(*(++HashMap<String, int>::Iterator(myMap3.begin())) == 122);
    ASSERT(myMap3.back() == 123);

    // test clear
    myMap.clear();
    ASSERT(myMap.size() == 0);
    ASSERT(myMap.isEmpty());

    // test front and back
    myMap.clear();
    myMap.append("1", 1);
    myMap.append("2", 2);
    ASSERT(myMap.front() == 1);
    ASSERT(myMap.back() == 2);
    myMap.append("3", 3);
    ASSERT(myMap.front() == 1);
    ASSERT(myMap.back() == 3);
    myMap.prepend("0", 0);
    ASSERT(myMap.front() == 0);
    ASSERT(myMap.back() == 3);

    // test remove front and back
    myMap.clear();
    myMap.append("1", 1);
    myMap.append("2", 2);
    myMap.append("3", 3);
    myMap.removeFront();
    ASSERT(myMap.size() == 2);
    ASSERT(myMap.front() == 2);
    myMap.removeBack();
    ASSERT(myMap.size() == 1);
    ASSERT(myMap.front() == 2);
    ASSERT(myMap.back() == 2);
  }
}

int main(int argc, char* argv[])
{
    testHashMap();
    return 0;
}
