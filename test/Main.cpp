
#include <nstd/Console.h>
#include <nstd/Memory.h>
#include <nstd/Debug.h>
#include <nstd/Mutex.h>
#include <nstd/Atomic.h>
#include <nstd/String.h>
#include <nstd/HashSet.h>
#include <nstd/List.h>
#include <nstd/HashMap.h>

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
  Memory::free(buffer);
  buffer = Memory::alloc(123, size);
  ASSERT(size >= 123);
  Memory::free(buffer);
}

void_t testMemoryAllocLarge()
{
  size_t size;
  void_t* buffer = Memory::alloc(50000 * 5, size);
  ASSERT(buffer);
  ASSERT(size >= 123);
  Memory::free(buffer);
  buffer = Memory::alloc(50000 * 5, size);
  ASSERT(size >= 123);
  Memory::free(buffer);
}

void_t testConsolePrintf()
{
  Console::printf("%s\n", "Hello world");
  size_t bufferSize;
  char_t* buffer = (char_t*)Memory::alloc(5000 * 4, bufferSize);
  Memory::fill(buffer, bufferSize - 1, 'a');
  buffer[bufferSize - 2] = 'b';
  buffer[bufferSize - 1] = '\0';
  Console::printf("%s%s\n", buffer, buffer);
}

void_t testDebugPrintf()
{
  Debug::printf("%s\n", "Hello world");
  size_t bufferSize;
  char_t* buffer = (char_t*)Memory::alloc(5000 * 4, bufferSize);
  Memory::fill(buffer, bufferSize - 1, 'a');
  buffer[bufferSize - 2] = 'b';
  buffer[bufferSize - 1] = '\0';
  Debug::printf("%s%s\n", buffer, buffer);
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
  volatile uint64_t uint64 =  (0xffffffffLL << 32) | 0xfffffff0LL;
  ASSERT(Atomic::increment(uint64) == ((0xffffffffLL << 32) | 0xfffffff1LL));
  ASSERT(Atomic::decrement(uint64) == ((0xffffffffLL << 32) | 0xfffffff0LL));
}

#include <cctype>
void_t testString()
{
  String empty;
  String hello("hello");
  String copyOfHello(hello);
  String copyOfCopyOfHello(copyOfHello);
  ASSERT(hello == copyOfCopyOfHello);
  ASSERT(copyOfHello == copyOfCopyOfHello);
  ASSERT(hello == copyOfCopyOfHello);
  ASSERT(hello != empty);
  ASSERT(copyOfHello != empty);
  ASSERT(copyOfCopyOfHello != empty);
  ASSERT(!(hello == empty));
  ASSERT(!(copyOfHello == empty));
  ASSERT(!(copyOfCopyOfHello == empty));
  copyOfHello.clear();
  empty.printf("%s %s", (const char_t*)hello, "world");
  ASSERT(empty == "hello world");
  ASSERT(empty != "hello worl2");
  ASSERT(empty != "hello worl2a");

  for (int i = 0; i < 0x100; ++i)
  {
    ASSERT(String::toUpperCase((char_t)i) == toupper((char_t&)i));
    ASSERT(String::toLowerCase((char_t)i) == tolower((char_t&)i));
    ASSERT(String::isSpace((char_t)i) == !!isspace((char_t&)i));
  }
}

#include <cstdlib>
void_t testHashSet()
{
  HashSet<int_t> mySet;
  ASSERT(mySet.empty());
  ASSERT(mySet.begin() == mySet.end());
  mySet.insert(1);
  mySet.insert(2);
  mySet.insert(3);
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
  mySet.insert(5);
  mySet.insert(6);
  ASSERT(mySet.find(3) == mySet.end());
  ASSERT(mySet.find(1) == mySet.end());
  ASSERT(mySet.find(5) != mySet.end());
  ASSERT(mySet.find(6) != mySet.end());
  it = mySet.begin();
  ASSERT(*it == 2);
  ASSERT(*(++it) == 5);
  ASSERT(*(++it) == 6);
  for(int i = 0; i < 300; ++i)
    mySet.insert((rand() % 30) + 10);
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
  mySet.insert(400);
  mySet.insert(400);
  mySet.remove(400);
  ASSERT(mySet.find(400) == mySet.end());
  mySet.clear();
  ASSERT(mySet.size() == 0);
  ASSERT(mySet.empty());
}

void_t testHashSetDestructor()
{
  static int_t destructions = 0;
  struct Test
  {
    int_t num;
    //int_t* destructions;

    Test() : num(0) {}
    Test(int_t num) : num(num) {}
    ~Test()
    {
      ++destructions;
    }

    operator size_t() const {return num;}
  };

  {
    HashSet<Test> mySet;
    mySet.insert(1);
    mySet.insert(2);
    mySet.insert(3);
    mySet.remove(1);
  }
  ASSERT(destructions == 8);
}

void_t testHashSetString()
{
  HashSet<String> mySet;
  ASSERT(mySet.empty());
  mySet.insert("what");
  mySet.insert("bv");
  mySet.insert("c");
  ASSERT(mySet.find("what") != mySet.end());
  ASSERT(mySet.find("wdashat") == mySet.end());
}

void_t testList()
{
  List<String> myList;
  ASSERT(myList.empty());
  myList.append("string1");
  myList.append("string2");
  myList.append("string3");
  ASSERT(myList.size() == 3);
  ASSERT(myList.find("string2") != myList.end());
  ASSERT(myList.find("string4") == myList.end());
  myList.remove("string2");
  ASSERT(myList.size() == 2);
  ASSERT(myList.find("string2") == myList.end());
  List<String>::Iterator it = myList.begin();
  ASSERT(*it == "string1");
  ASSERT(*(++it) == "string3");
  *it = "abbba";
  ASSERT(*it == "abbba");
  myList.prepend("string7");
  ASSERT(*myList.begin() == "string7");
  myList.clear();
  ASSERT(myList.size() == 0);
  ASSERT(myList.empty());
}

void_t testHashMap()
{
  HashMap<String, int_t> myMap;
  ASSERT(myMap.empty());
  myMap.insert("123", 123);
  myMap.insert("123", 125);
  ASSERT(*myMap.find("123") == 125);
  myMap.clear();
  ASSERT(myMap.size() == 0);
  ASSERT(myMap.empty());
}

int_t main(int_t argc, char_t* argv[])
{
  Console::printf("%s\n", "Testing..."); 

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
  testHashMap();

  Console::printf("%s\n", "done"); 

  return 0;
}
