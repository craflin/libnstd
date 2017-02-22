
#include <nstd/Console.h>
#include <nstd/Memory.h>
#include <nstd/Debug.h>
#include <nstd/Mutex.h>
#include <nstd/String.h>
#include <nstd/HashSet.h>
#include <nstd/List.h>
#include <nstd/HashMap.h>
#include <nstd/Array.h>
#include <nstd/Thread.h>
#include <nstd/Time.h>
#include <nstd/Error.h>
#include <nstd/Variant.h>
#include <nstd/Math.h>

void testAtomic();
void testProcess();
void testMonitor();
void testSignal();
void testUnicode();
void testBuffer();
void testThread();
void testSempahore();
void testDirectory();
void testMutex();
void testPoolList();
void testPoolMap();
void testServer();
void testMultiMap();
void testError();
void testString();
void testMemory();
void testVariant();
void testFile();
void testXML();
void testJSON();
void testTime();
void testMap();

void testConsolePrintf()
{
  Console::printf(_T("%s\n"), _T("Hello world"));
  usize bufferSize;
  char* buffer = (char*)Memory::alloc(5000 * 4, bufferSize);
  Memory::fill(buffer, 'a', bufferSize - 1);
  buffer[bufferSize - 2] = 'b';
  buffer[bufferSize - 1] = '\0';
  Console::printf(_T("%hs%hs\n"), buffer, buffer);
}

void testDebugPrintf()
{
  Debug::printf(_T("%s\n"), _T("Hello world"));
  usize bufferSize;
  char* buffer = (char*)Memory::alloc(5000 * 4, bufferSize);
  Memory::fill(buffer, 'a', bufferSize - 1);
  buffer[bufferSize - 2] = 'b';
  buffer[bufferSize - 1] = '\0';
  Debug::printf(_T("%hs%hs\n"), buffer, buffer);
}

void testHashSet()
{
  HashSet<int> mySet;
  HashSet<int> emptySet;
  mySet.swap(emptySet);
  ASSERT(mySet.isEmpty());
  ASSERT(mySet.begin() == mySet.end());
  mySet.append(1);
  ASSERT(mySet.contains(1));
  mySet.append(2);
  mySet.append(3);
  ASSERT(mySet.begin() != mySet.end());
  HashSet<int>::Iterator it = mySet.begin();
  ASSERT(*it == 1);
  ASSERT(*(++it) == 2);
  ASSERT(*(++it) == 3);
  ASSERT(++it == mySet.end());
  ASSERT(mySet.size() == 3);
  ASSERT(mySet.find(2) != mySet.end());
  ASSERT(mySet.find(5) == mySet.end());
  mySet.remove(3);
  mySet.remove(1);
  mySet.append(5);
  mySet.append(6);
  ASSERT(mySet.find(3) == mySet.end());
  ASSERT(mySet.find(1) == mySet.end());
  ASSERT(mySet.find(5) != mySet.end());
  ASSERT(mySet.find(6) != mySet.end());
  it = mySet.begin();
  ASSERT(*it == 2);
  ASSERT(*(++it) == 5);
  ASSERT(*(++it) == 6);
  for(int i = 0; i < 300; ++i)
    mySet.append((Math::random() % 30) + 10);
  for(int i = 0; i < 300; ++i)
    mySet.remove((Math::random() % 30) + 10);
  for(HashSet<int>::Iterator it = mySet.begin(), end = mySet.end(), next; it != end; it = next)
  {
    next = it; ++next;
    if(*it >= 10)
      mySet.remove(it);
  }
  it = mySet.begin();
  ASSERT(*it == 2);
  ASSERT(*(++it) == 5);
  ASSERT(*(++it) == 6);
  mySet.append(400);
  mySet.append(400);
  mySet.remove(400);
  ASSERT(mySet.find(400) == mySet.end());
  mySet.clear();
  ASSERT(mySet.size() == 0);
  ASSERT(mySet.isEmpty());

  // test front and back
  mySet.clear();
  mySet.append(1);
  mySet.append(2);
  ASSERT(mySet.front() == 1);
  ASSERT(mySet.back() == 2);

  // test remove front and back
  mySet.clear();
  mySet.append(1);
  mySet.append(2);
  mySet.append(3);
  mySet.removeFront();
  ASSERT(mySet.size() == 2);
  ASSERT(mySet.front() == 2);
  mySet.removeBack();
  ASSERT(mySet.size() == 1);
  ASSERT(mySet.front() == 2);
  ASSERT(mySet.back() == 2);
}

struct TestHashSetDestructor
{
  int num;
  //int* destructions;

  TestHashSetDestructor() : num(0) {}
  TestHashSetDestructor(int num) : num(num) {}
  ~TestHashSetDestructor()
  {
    ++destructions;
  }

  operator usize() const {return num;}

  static int destructions;
};

int TestHashSetDestructor::destructions = 0;

void testHashSetDestructor()
{
  {
    HashSet<TestHashSetDestructor> mySet;
    mySet.append(1);
    mySet.append(2);
    mySet.append(3);
    mySet.remove(1);
    ASSERT(mySet.size() == 2);
  }
  ASSERT(TestHashSetDestructor::destructions == 8);
}

void testHashSetString()
{
  // test append
  HashSet<String> mySet;
  ASSERT(mySet.isEmpty());
  mySet.append(_T("string1"));
  mySet.append(_T("string2"));
  mySet.append(_T("string3"));
  ASSERT(mySet.size() == 3);
  ASSERT(!mySet.isEmpty());

  // test find
  ASSERT(mySet.find(_T("string1")) != mySet.end());
  ASSERT(mySet.find(_T("string2")) != mySet.end());
  ASSERT(*mySet.find(_T("string2")) == _T("string2"));
  ASSERT(mySet.find(_T("string3")) != mySet.end());
  ASSERT(mySet.find(_T("wdashat")) == mySet.end());

  // test list copy constructor
  HashSet<String> mySet2(mySet);
  ASSERT(mySet2.size() == 3);
  ASSERT(*mySet2.begin() == _T("string1"));
  ASSERT(*(++HashSet<String>::Iterator(mySet2.begin())) == _T("string2"));
  ASSERT(mySet2.back() == _T("string3"));

  // test list copy operator
  HashSet<String> mySet3;
  mySet3 = mySet;
  ASSERT(mySet3.size() == 3);
  ASSERT(*mySet3.begin() == _T("string1"));
  ASSERT(*(++HashSet<String>::Iterator(mySet3.begin())) == _T("string2"));
  ASSERT(mySet3.back() == _T("string3"));
}

void testList()
{
  // test list append
  List<String> myList;
  List<String> emptyList;
  myList.swap(emptyList);
  ASSERT(myList.isEmpty());
  myList.append(_T("string1"));
  myList.append(_T("string2"));
  myList.append(_T("string3"));
  ASSERT(myList.size() == 3);

  // test list copy constructor
  List<String> myList2(myList);
  ASSERT(myList2.size() == 3);
  ASSERT(*myList2.begin() == _T("string1"));
  ASSERT(*(++List<String>::Iterator(myList2.begin())) == _T("string2"));
  ASSERT(*(++myList2.begin()) == _T("string2"));
  ASSERT(myList2.back() == _T("string3"));

  // test list copy operator
  List<String> myList3;
  myList3 = myList;
  ASSERT(myList3.size() == 3);
  ASSERT(*myList3.begin() == _T("string1"));
  ASSERT(*(++List<String>::Iterator(myList3.begin())) == _T("string2"));
  ASSERT(myList3.back() == _T("string3"));

  // test list find
  ASSERT(myList.find(_T("string2")) != myList.end());
  ASSERT(myList.find(_T("string4")) == myList.end());
  myList.remove(_T("string2"));
  ASSERT(myList.size() == 2);
  ASSERT(myList.find(_T("string2")) == myList.end());

  // test list iterator
  List<String>::Iterator it = myList.begin();
  ASSERT(*it == _T("string1"));
  ASSERT(*(++it) == _T("string3"));
  *it = _T("abbba");
  ASSERT(*it == _T("abbba"));

  // test prepend
  myList.prepend(_T("string7"));
  ASSERT(*myList.begin() == _T("string7"));

  // test list clear
  myList.clear();
  ASSERT(myList.size() == 0);
  ASSERT(myList.isEmpty());

  // test front and back
  myList.clear();
  myList.append(_T("1"));
  myList.append(_T("2"));
  ASSERT(myList.front() == _T("1"));
  ASSERT(myList.back() == _T("2"));
  myList.append(_T("3"));
  ASSERT(myList.front() == _T("1"));
  ASSERT(myList.back() == _T("3"));
  myList.prepend(_T("0"));
  ASSERT(myList.front() == _T("0"));
  ASSERT(myList.back() == _T("3"));

  // test remove front and back
  myList.clear();
  myList.append(_T("1"));
  myList.append(_T("2"));
  myList.append(_T("3"));
  myList.removeFront();
  ASSERT(myList.size() == 2);
  ASSERT(myList.front() == _T("2"));
  myList.removeBack();
  ASSERT(myList.size() == 1);
  ASSERT(myList.front() == _T("2"));
  ASSERT(myList.back() == _T("2"));
}

void testListSort()
{
  List<int> myList;
  for(int i = 0; i < 100; ++i)
    myList.append(Math::random() % 90);
  myList.sort();
  int current = 0;
  for(List<int>::Iterator i = myList.begin(), end = myList.end(); i != end; ++i)
  {
    ASSERT(*i >= current);
    current = *i;
  }
}

void testListStringSort()
{
  List<String> myList;
  String str;
  for(int i = 0; i < 100; ++i)
  {
    str.printf(_T("abc%d"), (int)(Math::random() % 90));
    myList.append(str);
  }
  myList.sort();
  String current(_T("abc0"));
  for(List<String>::Iterator i = myList.begin(), end = myList.end(); i != end; ++i)
  {
    ASSERT(*i >= current);
    current = *i;
  }
}

void testHashMap()
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

void testHashMapString()
{
  // test append
  HashMap<String, int> myMap;
  ASSERT(myMap.isEmpty());
  myMap.append(_T("string1"), 120);
  myMap.append(_T("string1"), 121);
  ASSERT(myMap.size() == 1);
  myMap.append(_T("string2"), 122);
  myMap.append(_T("string3"), 123);
  ASSERT(myMap.size() == 3);

  // test find
  ASSERT(*myMap.find(_T("string1")) == 121);
  ASSERT(*myMap.find(_T("string2")) == 122);
  ASSERT(*myMap.find(_T("string3")) == 123);
  ASSERT(myMap.find(_T("dsadasa")) == myMap.end());

  // test list copy constructor
  HashMap<String, int> myMap2(myMap);
  ASSERT(myMap2.size() == 3);
  ASSERT(*myMap2.begin() == 121);
  ASSERT(myMap2.begin().key() == _T("string1"));
  ASSERT(*(++HashMap<String, int>::Iterator(myMap2.begin())) == 122);
  ASSERT(myMap2.back() == 123);

  // test list copy operator
  HashMap<String, int> myMap3;
  myMap3 = myMap;
  ASSERT(myMap3.size() == 3);
  ASSERT(*myMap3.begin() == 121);
  ASSERT(myMap3.begin().key() == _T("string1"));
  ASSERT(*(++HashMap<String, int>::Iterator(myMap3.begin())) == 122);
  ASSERT(myMap3.back() == 123);

  // test clear
  myMap.clear();
  ASSERT(myMap.size() == 0);
  ASSERT(myMap.isEmpty());

  // test front and back
  myMap.clear();
  myMap.append(_T("1"), 1);
  myMap.append(_T("2"), 2);
  ASSERT(myMap.front() == 1);
  ASSERT(myMap.back() == 2);
  myMap.append(_T("3"), 3);
  ASSERT(myMap.front() == 1);
  ASSERT(myMap.back() == 3);
  myMap.prepend(_T("0"), 0);
  ASSERT(myMap.front() == 0);
  ASSERT(myMap.back() == 3);

  // test remove front and back
  myMap.clear();
  myMap.append(_T("1"), 1);
  myMap.append(_T("2"), 2);
  myMap.append(_T("3"), 3);
  myMap.removeFront();
  ASSERT(myMap.size() == 2);
  ASSERT(myMap.front() == 2);
  myMap.removeBack();
  ASSERT(myMap.size() == 1);
  ASSERT(myMap.front() == 2);
  ASSERT(myMap.back() == 2);
}

void testNewDelete()
{
  static uint constructorCalls = 0;
  static uint destructorCalls = 0;
  class MyClass
  {
  public:
    MyClass()
    {
      if(constructorCalls == 0)
      {
        ASSERT(Memory::size(this) >= sizeof(MyClass));
      }
      ++constructorCalls;
    };
    ~MyClass() {++destructorCalls;};
    String aaaa;
  };

  MyClass* aa = new MyClass;
  delete aa;

  MyClass* bb = new MyClass[23];
  delete [] bb;

  ASSERT(constructorCalls == 24);
  ASSERT(destructorCalls == 24);
}

void testArray()
{
  // test append
  Array<int> myArray;
  Array<int> emptyArray;
  myArray.swap(emptyArray);
  ASSERT(myArray.isEmpty());
  ASSERT(myArray.size() == 0);
  ASSERT(myArray.append(123) == 123);
  ASSERT(myArray.append(124) == 124);
  ASSERT(myArray.append(125) == 125);
  ASSERT(!myArray.isEmpty());
  ASSERT(myArray.size() == 3);

  // tets clear
  myArray.clear();
  ASSERT(myArray.isEmpty());
  ASSERT(myArray.size() == 0);
}

void testArrayString()
{
  // test insert
  Array<String> myArray;
  ASSERT(myArray.isEmpty());
  ASSERT(myArray.size() == 0);
  ASSERT(myArray.append(_T("string1")) == _T("string1"));
  ASSERT(myArray.append(_T("string2")) == _T("string2"));
  ASSERT(myArray.append(_T("string3")) == _T("string3"));
  ASSERT(!myArray.isEmpty());
  ASSERT(myArray.size() == 3);

  // test list copy constructor
  Array<String> myArray2(myArray);
  ASSERT(myArray2.size() == 3);
  ASSERT(*myArray2.begin() == _T("string1"));
  ASSERT(*(++Array<String>::Iterator(myArray2.begin())) == _T("string2"));
  ASSERT(myArray2.back() == _T("string3"));

  // test list copy operator
  Array<String> myArray3;
  myArray3 = myArray;
  ASSERT(myArray3.size() == 3);
  ASSERT(*myArray3.begin() == _T("string1"));
  ASSERT(*(++Array<String>::Iterator(myArray3.begin())) == _T("string2"));
  ASSERT(myArray3.back() == _T("string3"));

  // test clear
  myArray.clear();
  ASSERT(myArray.isEmpty());
  ASSERT(myArray.size() == 0);

  // test iterator
  String str;
  for(int i = 0; i < 500; ++i)
  {
    str.printf(_T("test%d"), i);
    myArray.append(str);
  }
  ASSERT(myArray.size() == 500);
  int count = 0;
  for(Array<String>::Iterator i = myArray.begin(), end = myArray.end(); i != end; ++i)
  {
    str.printf(_T("test%d"), count);
    ASSERT(*i == str);
    ++count;
  }
  ASSERT(count == 500);

  // test remove
  myArray.remove(23);
  ASSERT(myArray.size() == 499);
  ASSERT(myArray.remove(myArray.begin()) == myArray.begin());

  // test reserve
  myArray.clear();
  myArray.append(_T("test1"));
  myArray.reserve(1000);
  ASSERT(myArray.capacity() >= 1000);
  ASSERT(myArray.size() == 1);
  ASSERT(myArray[0] == _T("test1"));

  // test resize
  myArray.clear();
  for (int i = 0; i < 100; ++i)
    myArray.append(_T("test"));
  ASSERT(myArray.size() == 100);
  myArray.resize(110, _T("dasda"));
  ASSERT(myArray.size() == 110);
  myArray.resize(90);
  ASSERT(myArray.size() == 90);

  // test front and back
  myArray.clear();
  myArray.append(_T("1"));
  myArray.append(_T("2"));
  ASSERT(myArray.front() == _T("1"));
  ASSERT(myArray.back() == _T("2"));
  myArray.append(_T("3"));
  ASSERT(myArray.front() == _T("1"));
  ASSERT(myArray.back() == _T("3"));

  // test remove front and back
  myArray.clear();
  myArray.append(_T("1"));
  myArray.append(_T("2"));
  myArray.append(_T("3"));
  myArray.removeFront();
  ASSERT(myArray.size() == 2);
  ASSERT(myArray.front() == _T("2"));
  myArray.removeBack();
  ASSERT(myArray.size() == 1);
  ASSERT(myArray.front() == _T("2"));
  ASSERT(myArray.back() == _T("2"));
}

int main(int argc, char* argv[])
{
  Console::printf(_T("%s\n"), _T("Testing..."));

  /*
  testMemory();
  */
  testServer();
  /*
  testSignal();
  testMonitor();
  testUnicode();
  testBuffer();
  testThread();
  testSempahore();
  testMutex();
  //testConsolePrintf();
  //testDebugPrintf();
  testAtomic();
  testString();
  testHashSet();
  testHashSetDestructor();
  testHashSetString();
  testList();
  testListSort();
  testListStringSort();
  testHashMap();
  testHashMapString();
  testNewDelete();
  testFile();
  testArray();
  testArrayString();
  testDirectory();
  testTime();
  testProcess();
  testMap();
  testPoolList();
  testPoolMap();
  testMultiMap();
  testError();
  testVariant();
  testXML();
  testJSON();
  */

  Console::printf(_T("%s\n"), _T("done"));

  return 0;
}
