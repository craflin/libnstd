
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
void testConsole();
void testDebug();
void testProcess();
void testMonitor();
void testSignal();
void testUnicode();
void testHashSet();
void testBuffer();
void testThread();
void testSempahore();
void testDirectory();
void testMutex();
void testPoolList();
void testPoolMap();
void testSocket();
void testServer();
void testMultiMap();
void testError();
void testString();
void testMemory();
void testVariant();
void testFile();
void testXML();
void testJSON();
void testCallback();
void testTime();
void testMap();
void testSha256();
void testMap();
void testRefCount(); 
void testList();

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

  testMemory();
  testSha256();
  testSocket();
  testServer();
  testSignal();
  testMonitor();
  testUnicode();
  testBuffer();
  testThread();
  testSempahore();
  testMutex();
  //testConsole();
  //testDebug();
  testAtomic();
  testString();
  testHashSet();
  testList();
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
  testCallback();
  testRefCount();

  Console::printf(_T("%s\n"), _T("done"));

  return 0;
}
