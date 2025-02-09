
#include <nstd/Array.hpp>
#include <nstd/Debug.hpp>
#include <nstd/String.hpp>

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
  ASSERT(myArray.append("string1") == "string1");
  ASSERT(myArray.append("string2") == "string2");
  ASSERT(myArray.append("string3") == "string3");
  ASSERT(!myArray.isEmpty());
  ASSERT(myArray.size() == 3);

  // test list copy constructor
  Array<String> myArray2(myArray);
  ASSERT(myArray2.size() == 3);
  ASSERT(*myArray2.begin() == "string1");
  ASSERT(*(++Array<String>::Iterator(myArray2.begin())) == "string2");
  ASSERT(myArray2.back() == "string3");

  // test list copy operator
  Array<String> myArray3;
  myArray3 = myArray;
  ASSERT(myArray3.size() == 3);
  ASSERT(*myArray3.begin() == "string1");
  ASSERT(*(++Array<String>::Iterator(myArray3.begin())) == "string2");
  ASSERT(myArray3.back() == "string3");

  // test clear
  myArray.clear();
  ASSERT(myArray.isEmpty());
  ASSERT(myArray.size() == 0);

  // test iterator
  String str;
  for(int i = 0; i < 500; ++i)
  {
    str.printf("test%d", i);
    myArray.append(str);
  }
  ASSERT(myArray.size() == 500);
  int count = 0;
  for(Array<String>::Iterator i = myArray.begin(), end = myArray.end(); i != end; ++i)
  {
    str.printf("test%d", count);
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
  myArray.append("test1");
  myArray.reserve(1000);
  ASSERT(myArray.capacity() >= 1000);
  ASSERT(myArray.size() == 1);
  ASSERT(myArray[0] == "test1");

  // test resize
  myArray.clear();
  for (int i = 0; i < 100; ++i)
    myArray.append("test");
  ASSERT(myArray.size() == 100);
  myArray.resize(110, "dasda");
  ASSERT(myArray.size() == 110);
  myArray.resize(90);
  ASSERT(myArray.size() == 90);

  // test front and back
  myArray.clear();
  myArray.append("1");
  myArray.append("2");
  ASSERT(myArray.front() == "1");
  ASSERT(myArray.back() == "2");
  myArray.append("3");
  ASSERT(myArray.front() == "1");
  ASSERT(myArray.back() == "3");

  // test remove front and back
  myArray.clear();
  myArray.append("1");
  myArray.append("2");
  myArray.append("3");
  myArray.removeFront();
  ASSERT(myArray.size() == 2);
  ASSERT(myArray.front() == "2");
  myArray.removeBack();
  ASSERT(myArray.size() == 1);
  ASSERT(myArray.front() == "2");
  ASSERT(myArray.back() == "2");
}

int main(int argc, char* argv[])
{
    testArray();
    testArrayString();
    return 0;
}
