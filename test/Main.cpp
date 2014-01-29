
#include <nstd/Console.h>
#include <nstd/Memory.h>
#include <nstd/Debug.h>
#include <nstd/Mutex.h>
#include <nstd/Atomic.h>
#include <nstd/String.h>
#include <nstd/HashSet.h>
#include <nstd/List.h>
#include <nstd/HashMap.h>
#include <nstd/File.h>
#include <nstd/Directory.h>
#include <nstd/Array.h>
#include <nstd/Thread.h>
#include <nstd/Semaphore.h>
#include <nstd/Time.h>
#include <nstd/Error.h>
#include <nstd/Buffer.h>
#include <nstd/Variant.h>

#include <cstring>
#include <cctype>
#include <cstdlib>

void_t testThread()
{
  Thread thread;
  Thread thread2;
  Thread thread3;
  struct ThreadData
  {
    static uint_t proc(void_t* args)
    {
      ThreadData& threadData = *(ThreadData*)args;

      ASSERT(threadData.intParam == 123);
      for (int i = 0; i < 10000; ++i)
      {
        Atomic::increment(threadData.counter);
        Thread::yield();
      }
      return 42;
    }
    uint_t intParam;
    volatile uint_t counter;
  } threadData;
  threadData.intParam = 123;
  threadData.counter = 0;
  ASSERT(thread.start(ThreadData::proc, &threadData));
  ASSERT(thread2.start(ThreadData::proc, &threadData));
  ASSERT(thread3.start(ThreadData::proc, &threadData));
  ASSERT(thread.join() == 42);
  ASSERT(thread.start(ThreadData::proc, &threadData));
  ASSERT(thread.join() == 42);
  ASSERT(thread2.join() == 42);
  ASSERT(thread3.join() == 42);
  ASSERT(threadData.counter == 4 * 10000);
}

void_t testSempahore()
{
  Semaphore sem(3);
  sem.signal();
  ASSERT(sem.wait());
  ASSERT(sem.wait());
  ASSERT(sem.wait());
  ASSERT(sem.wait());
  ASSERT(!sem.tryWait());
  timestamp_t start = Time::ticks();
  ASSERT(!sem.wait(300));
  timestamp_t waitTime = Time::ticks() - start;
  ASSERT(waitTime > 200);
}

void_t testMutexRecursion()
{
  Mutex mutex;
  mutex.lock();
  mutex.lock();
  mutex.unlock();
  mutex.unlock();
}

void_t testMemoryAllocSmall()
{
  size_t size;
  void_t* buffer = Memory::alloc(123, size);
  ASSERT(buffer);
  ASSERT(size >= 123);
  ASSERT(Memory::size(buffer) == size);
  Memory::free(buffer);
  buffer = Memory::alloc(123, size);
  ASSERT(size >= 123);
  
  char_t testBuffer[100];
  memset(testBuffer, 'a', sizeof(testBuffer));
  Memory::fill(buffer, 'b', size);
  ASSERT(Memory::compare(buffer, testBuffer, sizeof(testBuffer)) != 0);
  ASSERT(Memory::compare(buffer, testBuffer, sizeof(testBuffer)) == memcmp(buffer, testBuffer, sizeof(testBuffer)));
  ASSERT(Memory::compare(testBuffer, buffer, sizeof(testBuffer)) == memcmp(testBuffer, buffer, sizeof(testBuffer)));
  Memory::fill(buffer, 'a', size);
  ASSERT(Memory::compare(buffer, testBuffer, sizeof(testBuffer)) == 0);

  Memory::free(buffer);
}

void_t testMemoryAllocLarge()
{
  size_t size;
  void_t* buffer = Memory::alloc(50000 * 5, size);
  ASSERT(buffer);
  ASSERT(size >= 50000 * 5);
  Memory::free(buffer);
  buffer = Memory::alloc(50000 * 5, size);
  ASSERT(size >= 50000 * 5);
  Memory::free(buffer);
}

void_t testConsolePrintf()
{
  Console::printf(_T("%s\n"), _T("Hello world"));
  size_t bufferSize;
  char_t* buffer = (char_t*)Memory::alloc(5000 * 4, bufferSize);
  Memory::fill(buffer, 'a', bufferSize - 1);
  buffer[bufferSize - 2] = 'b';
  buffer[bufferSize - 1] = '\0';
  Console::printf(_T("%hs%hs\n"), buffer, buffer);
}

void_t testDebugPrintf()
{
  Debug::printf(_T("%s\n"), _T("Hello world"));
  size_t bufferSize;
  char_t* buffer = (char_t*)Memory::alloc(5000 * 4, bufferSize);
  Memory::fill(buffer, 'a', bufferSize - 1);
  buffer[bufferSize - 2] = 'b';
  buffer[bufferSize - 1] = '\0';
  Debug::printf(_T("%hs%hs\n"), buffer, buffer);
}

void_t testAtomic()
{
  volatile size_t size = 0;
  ASSERT(Atomic::increment(size) == 1);
  ASSERT(Atomic::increment(size) == 2);
  ASSERT(Atomic::increment(size) == 3);
  ASSERT(Atomic::decrement(size) == 2);
  ASSERT(Atomic::decrement(size) == 1);
  ASSERT(Atomic::decrement(size) == 0);
  volatile int32_t int32 = 0;
  ASSERT(Atomic::increment(int32) == 1);
  ASSERT(Atomic::decrement(int32) == 0);
  ASSERT(Atomic::decrement(int32) == -1);
  volatile uint32_t uint32 = 0xfffffff0;
  ASSERT(Atomic::increment(uint32) == 0xfffffff1);
  ASSERT(Atomic::decrement(uint32) == 0xfffffff0);
  volatile int64_t int64 = 0;
  ASSERT(Atomic::increment(int64) == 1);
  ASSERT(Atomic::decrement(int64) == 0);
  ASSERT(Atomic::decrement(int64) == -1);
  volatile uint64_t uint64 =  (0xffffffffULL << 32) | 0xfffffff0ULL;
  ASSERT(Atomic::increment(uint64) == ((0xffffffffULL << 32) | 0xfffffff1ULL));
  ASSERT(Atomic::decrement(uint64) == ((0xffffffffULL << 32) | 0xfffffff0ULL));
}

void_t testString()
{
  // test constructors
  String empty;
  ASSERT(empty.isEmpty());
  String hello(_T("hello"));
  String copyOfHello(hello);
  String copyOfCopyOfHello(copyOfHello);

  // test compare operators
  ASSERT(hello == copyOfCopyOfHello);
  ASSERT(copyOfHello == copyOfCopyOfHello);
  ASSERT(hello == copyOfCopyOfHello);
  ASSERT(hello != empty);
  ASSERT(copyOfHello != empty);
  ASSERT(copyOfCopyOfHello != empty);
  ASSERT(!(hello == empty));
  ASSERT(!(copyOfHello == empty));
  ASSERT(!(copyOfCopyOfHello == empty));

  // test clear
  copyOfHello.clear();
  ASSERT(copyOfHello.isEmpty());

  // test printf
  empty.printf(_T("%s %s"), (const tchar_t*)hello, _T("world"));
  ASSERT(empty == _T("hello world"));
  ASSERT(empty != _T("hello worl2"));
  ASSERT(empty != _T("hello worl2a"));

  // test toUpperCase, toLowerCase, isSpace
  for (int i = 0; i < 0x100; ++i)
  {
#ifdef _UNICODE
    ASSERT(String::toUpperCase((wchar_t)i) == (wchar_t)towupper(i));
    ASSERT(String::toLowerCase((wchar_t)i) == (wchar_t)towlower(i));
    ASSERT(String::isSpace((wchar_t)i) == !!iswspace(i));
#else
    ASSERT(String::toUpperCase((char_t)i) == (char_t)toupper((uchar_t&)i));
    ASSERT(String::toLowerCase((char_t)i) == (char_t)tolower((uchar_t&)i));
    ASSERT(String::isSpace((char_t)i) == !!isspace((uchar_t&)i));
#endif
  }

  // test static length
  ASSERT(String::length(_T("")) == 0);
  ASSERT(String::length(_T("123")) == 3);

  // test find methods
  String test(_T("this is the find test test string"));
  ASSERT(String::compare(test.find(_T('i')), _T("is is the find test test string")) == 0);
  ASSERT(String::compare(test.findLast(_T('i')), _T("ing")) == 0);
  ASSERT(String::compare(test.find(_T("is")), _T("is is the find test test string")) == 0);
  ASSERT(String::compare(test.findLast(_T("is")), _T("is the find test test string")) == 0);
  ASSERT(String::compare(test.findOneOf(_T("ex")), _T("e find test test string")) == 0);
  ASSERT(String::compare(test.findOneOf(_T("xe")), _T("e find test test string")) == 0);
  ASSERT(String::compare(test.findLastOf(_T("ex")), _T("est string")) == 0);
  ASSERT(String::compare(test.findLastOf(_T("xe")), _T("est string")) == 0);

  // test prepend, append
  String b(_T(" b "));
  String a(_T("aa"));
  String c(_T("cc"));
  ASSERT(a + b + c == _T("aa b cc"));
  ASSERT(String().append(a).append(b).append(c) == _T("aa b cc"));
  ASSERT(String().append(b).prepend(a).append(c) == _T("aa b cc"));

  // test if lazy copying
  struct LazyCopyTest
  {
    static String test1()
    {
      return String(_T("test"));
    }
  };
  String aa = LazyCopyTest::test1(); // so, this is equal to "String aa(_T("test"))"?
  const tchar_t* caa = aa;
  ASSERT(caa != (tchar_t*)aa);

  // test external buffer attaching
  tchar_t buf[100];
  Memory::fill(buf, 'a', 8);
  String bufWrapper;
  bufWrapper.attach(buf, 4);
  ASSERT(bufWrapper == String(buf, 4));
}

void_t testHashSet()
{
  HashSet<int_t> mySet;
  ASSERT(mySet.isEmpty());
  ASSERT(mySet.begin() == mySet.end());
  mySet.append(1);
  mySet.append(2);
  mySet.append(3);
  ASSERT(mySet.begin() != mySet.end());
  HashSet<int_t>::Iterator it = mySet.begin();
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
    mySet.append((rand() % 30) + 10);
  for(int i = 0; i < 300; ++i)
    mySet.remove((rand() % 30) + 10);
  for(HashSet<int_t>::Iterator it = mySet.begin(), end = mySet.end(), next; it != end; it = next)
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
  int_t num;
  //int_t* destructions;

  TestHashSetDestructor() : num(0) {}
  TestHashSetDestructor(int_t num) : num(num) {}
  ~TestHashSetDestructor()
  {
    ++destructions;
  }

  operator size_t() const {return num;}

  static int_t destructions;
};

int_t TestHashSetDestructor::destructions = 0;

void_t testHashSetDestructor()
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

void_t testHashSetString()
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

void_t testList()
{
  // test list append
  List<String> myList;
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

void_t testListSort()
{
  List<int_t> myList;
  for(int_t i = 0; i < 100; ++i)
    myList.append(rand() % 90);
  myList.sort();
  int_t current = 0;
  for(List<int_t>::Iterator i = myList.begin(), end = myList.end(); i != end; ++i)
  {
    ASSERT(*i >= current);
    current = *i;
  }
}

void_t testListStringSort()
{
  List<String> myList;
  String str;
  for(int_t i = 0; i < 100; ++i)
  {
    str.printf(_T("abc%d"), (int_t)(rand() % 90));
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

void_t testHashMap()
{
  // test append
  HashMap<int_t, int_t> myMap;
  ASSERT(myMap.isEmpty());
  myMap.append(123, 123);
  myMap.append(123, 125);
  ASSERT(myMap.size() == 1);
  ASSERT(*myMap.find(123) == 125);

  // test clear
  myMap.clear();
  ASSERT(myMap.size() == 0);
  ASSERT(myMap.isEmpty());
}

void_t testHashMapString()
{
  // test append
  HashMap<String, int_t> myMap;
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
  HashMap<String, int_t> myMap2(myMap);
  ASSERT(myMap2.size() == 3);
  ASSERT(*myMap2.begin() == 121);
  ASSERT(myMap2.begin().key() == _T("string1"));
  ASSERT(*(++HashMap<String, int_t>::Iterator(myMap2.begin())) == 122);
  ASSERT(myMap2.back() == 123);

  // test list copy operator
  HashMap<String, int_t> myMap3;
  myMap3 = myMap;
  ASSERT(myMap3.size() == 3);
  ASSERT(*myMap3.begin() == 121);
  ASSERT(myMap3.begin().key() == _T("string1"));
  ASSERT(*(++HashMap<String, int_t>::Iterator(myMap3.begin())) == 122);
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

void_t testNewDelete()
{
  static uint_t constructorCalls = 0;
  static uint_t destructorCalls = 0;
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

void_t testFile()
{
  // test open and close
  {
    File file;
    ASSERT(!file.isOpen());
    ASSERT(file.open(_T("testfile.file.test"), File::writeFlag));
    ASSERT(file.isOpen());
    file.close();
    ASSERT(!file.isOpen());
    ASSERT(file.open(_T("testfile.file.test"), File::writeFlag));
    ASSERT(file.isOpen());
  }

  // test file exists
  ASSERT(File::exists(_T("testfile.file.test")));
  ASSERT(!File::exists(_T("dkasdlakshkalal.nonexisting.file")));

  // test file write
  char_t buffer[266];
  char_t buffer2[300];
  {
    File file;
    ASSERT(file.open(_T("testfile.file.test"), File::writeFlag));
    Memory::fill(buffer, 'a', sizeof(buffer));
    ASSERT(file.write(buffer, sizeof(buffer)) == sizeof(buffer));
    Memory::fill(buffer2, 'b', sizeof(buffer2));
    ASSERT(file.write(buffer2, sizeof(buffer2)) == sizeof(buffer2));
  }

  // test file read
  {
    File file;
    ASSERT(file.open(_T("testfile.file.test"), File::readFlag));
    char_t readBuffer[500];
    ASSERT(file.read(readBuffer, sizeof(readBuffer)) == sizeof(readBuffer));
    ASSERT(Memory::compare(readBuffer, buffer, sizeof(buffer)) == 0);
    ASSERT(Memory::compare(readBuffer + sizeof(buffer), buffer2, sizeof(readBuffer) - sizeof(buffer)) == 0);
    char_t readBuffer2[166];
    ASSERT(file.read(readBuffer2, sizeof(readBuffer2)) == sizeof(buffer) + sizeof(buffer2) - sizeof(readBuffer));
    ASSERT(Memory::compare(readBuffer2, buffer2 + sizeof(buffer2) - (sizeof(buffer) + sizeof(buffer2) - sizeof(readBuffer)), sizeof(buffer) + sizeof(buffer2) - sizeof(readBuffer)) == 0);
    file.close();
  }

  // test unlink
  ASSERT(File::unlink(_T("testfile.file.test")));
  ASSERT(!File::exists(_T("testfile.file.test")));
}

void_t testFileName()
{
  ASSERT(File::basename(_T("c:\\sadasd\\asdas\\test.blah")) == _T("test.blah"));
  ASSERT(File::basename(_T("c:\\sadasd\\asdas\\test")) == _T("test"));
  ASSERT(File::basename(_T("c:\\sadasd\\asdas\\test.blah"), _T("blah")) == _T("test"));
  ASSERT(File::basename(_T("c:\\sadasd\\asdas\\test.blah"), _T(".blah")) == _T("test"));
  ASSERT(File::extension(_T("c:\\sadasd\\asdas\\test.blah")) == _T("blah"));
  ASSERT(File::dirname(_T("c:\\sadasd\\asdas\\test.blah")) == _T("c:\\sadasd\\asdas"));
  ASSERT(File::dirname(_T("asdas/test.blah")) == _T("asdas"));

  ASSERT(File::simplifyPath(_T("../../dsadsad/2dsads")) == _T("../../dsadsad/2dsads"));
  ASSERT(File::simplifyPath(_T("..\\..\\dsadsad\\2dsads")) == _T("../../dsadsad/2dsads"));
  ASSERT(File::simplifyPath(_T(".././../dsadsad/2dsads")) == _T("../../dsadsad/2dsads"));
  ASSERT(File::simplifyPath(_T("dsadsad/../2dsads")) == _T("2dsads"));
  ASSERT(File::simplifyPath(_T("dsadsad/./../2dsads")) == _T("2dsads"));
  ASSERT(File::simplifyPath(_T("dsadsad/.././2dsads")) == _T("2dsads"));
  ASSERT(File::simplifyPath(_T("/dsadsad/../2dsads")) == _T("/2dsads"));
  ASSERT(File::simplifyPath(_T("/../../aaa/2dsads")) == _T("/../../aaa/2dsads"));

  ASSERT(File::isAbsolutePath(_T("/aaa/2dsads")));
  ASSERT(File::isAbsolutePath(_T("\\aaa\\2dsads")));
  ASSERT(File::isAbsolutePath(_T("c:/aaa/2dsads")));
  ASSERT(File::isAbsolutePath(_T("c:\\aaa\\2dsads")));
  ASSERT(!File::isAbsolutePath(_T("..\\aaa\\2dsads")));
  ASSERT(!File::isAbsolutePath(_T("aaa/2dsads")));
}

void_t testArray()
{
  // test append
  Array<int> myArray;
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


void_t testArrayString()
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
  for(int_t i = 0; i < 500; ++i)
  {
    str.printf(_T("test%d"), i);
    myArray.append(str);
  }
  ASSERT(myArray.size() == 500);
  int_t count = 0;
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
  for (int_t i = 0; i < 100; ++i)
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

void_t testError()
{
  File file;
  ASSERT(!file.open(_T("thisshouldbeanonexisting file")));
  ASSERT(!Error::getErrorString().isEmpty());
}

void_t testDirectory()
{
  // test create directory
  ASSERT(!Directory::exists(_T("testDir")));
  ASSERT(Directory::create(_T("testDir")));

  // test searching in an empty directory
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("*"), false));
    String path;
    bool_t isDir;
    ASSERT(!dir.read(path, isDir));
  }
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), String(), false));
    String path;
    bool_t isDir;
    ASSERT(!dir.read(path, isDir));
  }
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("a*"), false));
    String path;
    bool_t isDir;
    ASSERT(!dir.read(path, isDir));
  }

  // create some test files
  List<String> files;
  files.append(_T("ab"));
  files.append(_T("ab.t"));
  files.append(_T("ab.tx"));
  files.append(_T("ab.txt"));
  files.append(_T("ab.txtt"));
  for(List<String>::Iterator i = files.begin(), end = files.end(); i != end; ++i)
    ASSERT(File().open(String(_T("testDir/")) + *i, File::writeFlag));

  // create some test dirs
  List<String> dirs;
  dirs.append(_T("dirA"));
  dirs.append(_T("dirB"));
  for(List<String>::Iterator i = dirs.begin(), end = dirs.end(); i != end; ++i)
    ASSERT(Directory::create(String(_T("testDir/")) + *i));

  // search without pattern
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), String(), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    ASSERT(foundFiles == files);
    ASSERT(foundDirs == dirs);
  }

  // search for '*'
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("*"), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    ASSERT(foundFiles == files);
    ASSERT(foundDirs == dirs);
  }

  // search without pattern (dirs only)
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), String(), true));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    ASSERT(foundFiles.isEmpty());
    ASSERT(foundDirs == dirs);
  }

  // search for '*' (dirs only)
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("*"), true));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    ASSERT(foundFiles.isEmpty());
    ASSERT(foundDirs == dirs);
  }

  // seach for *a?t, should find ab.t
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("*b?t"), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    ASSERT(foundFiles.size() == 1 && foundFiles.front() == _T("ab.t"));
    ASSERT(foundDirs.isEmpty());
  }

  // search for *b.*, should not find ab
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("*b.*"), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    for(List<String>::Iterator i = foundFiles.begin(), end = foundFiles.end(); i != end; ++i)
      ASSERT(*i != _T("ab"));
    ASSERT(foundFiles.size() == 4);
    ASSERT(foundDirs.isEmpty());
  }

  // search for *.txt, should find ab.txt
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("*.txt"), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    ASSERT(foundFiles.size() == 1);
    ASSERT(foundDirs.isEmpty());
  }

  // search for a?.*
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("a?.*"), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    ASSERT(foundFiles.size() == 4);
    ASSERT(foundDirs.isEmpty());
  }

  // search for ab?.*
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("ab?.*"), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    ASSERT(foundFiles.isEmpty());
    ASSERT(foundDirs.isEmpty());
  }

  // delete test files
  for(List<String>::Iterator i = files.begin(), end = files.end(); i != end; ++i)
    ASSERT(File::unlink(String(_T("testDir/")) + *i));

  // delete test dirs
  for(List<String>::Iterator i = dirs.begin(), end = dirs.end(); i != end; ++i)
    ASSERT(Directory::unlink(String(_T("testDir/")) + *i));

  // delete test dir
  ASSERT(Directory::unlink(String(_T("testDir"))));
}

void_t testTime()
{
  String test = Time::toString(123 * 1000, "%Y-%m-%d %H:%M:%S");
  ASSERT(test == "1970-01-01 01:02:03");
}

int_t main(int_t argc, char_t* argv[])
{
  Console::printf(_T("%s\n"), _T("Testing..."));

  testThread();
  testSempahore();
  testMutexRecursion();
  testMemoryAllocSmall();
  testMemoryAllocLarge();
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
  testFileName();
  testArray();
  testArrayString();
  testError();
  testDirectory();
  testTime();

  Console::printf(_T("%s\n"), _T("done"));

  return 0;
}
