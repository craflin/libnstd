
#include <nstd/List.hpp>
#include <nstd/Debug.hpp>
#include <nstd/String.hpp>
#include <nstd/Math.hpp>

void testList()
{
  {
    // test list append
    List<String> myList;
    List<String> emptyList;
    myList.swap(emptyList);
    ASSERT(myList.isEmpty());
    myList.append("string1");
    myList.append("string2");
    myList.append("string3");
    ASSERT(myList.size() == 3);

    // test list copy constructor
    List<String> myList2(myList);
    ASSERT(myList2.size() == 3);
    ASSERT(*myList2.begin() == "string1");
    ASSERT(*(++List<String>::Iterator(myList2.begin())) == "string2");
    ASSERT(*(++myList2.begin()) == "string2");
    ASSERT(myList2.back() == "string3");

    // test list copy operator
    List<String> myList3;
    myList3 = myList;
    ASSERT(myList3.size() == 3);
    ASSERT(*myList3.begin() == "string1");
    ASSERT(*(++List<String>::Iterator(myList3.begin())) == "string2");
    ASSERT(myList3.back() == "string3");

    // test list find
    ASSERT(myList.find("string2") != myList.end());
    ASSERT(myList.find("string4") == myList.end());
    myList.remove("string2");
    ASSERT(myList.size() == 2);
    ASSERT(myList.find("string2") == myList.end());

    // test list iterator
    List<String>::Iterator it = myList.begin();
    ASSERT(*it == "string1");
    ASSERT(*(++it) == "string3");
    *it = "abbba";
    ASSERT(*it == "abbba");

    // test prepend
    myList.prepend("string7");
    ASSERT(*myList.begin() == "string7");

    // test list clear
    myList.clear();
    ASSERT(myList.size() == 0);
    ASSERT(myList.isEmpty());

    // test front and back
    myList.clear();
    myList.append("1");
    myList.append("2");
    ASSERT(myList.front() == "1");
    ASSERT(myList.back() == "2");
    myList.append("3");
    ASSERT(myList.front() == "1");
    ASSERT(myList.back() == "3");
    myList.prepend("0");
    ASSERT(myList.front() == "0");
    ASSERT(myList.back() == "3");

    // test remove front and back
    myList.clear();
    myList.append("1");
    myList.append("2");
    myList.append("3");
    myList.removeFront();
    ASSERT(myList.size() == 2);
    ASSERT(myList.front() == "2");
    myList.removeBack();
    ASSERT(myList.size() == 1);
    ASSERT(myList.front() == "2");
    ASSERT(myList.back() == "2");
  }

  // int sort
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

  // string sort
  {
    List<String> myList;
    String str;
    for(int i = 0; i < 100; ++i)
    {
      str.printf("abc%d", (int)(Math::random() % 90));
      myList.append(str);
    }
    myList.sort();
    String current("abc0");
    for(List<String>::Iterator i = myList.begin(), end = myList.end(); i != end; ++i)
    {
      ASSERT(*i >= current);
      current = *i;
    }
  }

  // insert
  {
    List<int> list;
    List<int>::Iterator it = list.insert(list.begin(), 1);
    ASSERT(it == list.begin());
    ASSERT(*it == 1);
    ASSERT(list.size() == 1);
  }
  {
    List<int> list;
    list.append(1);
    List<int>::Iterator it = list.insert(list.begin(), 2);
    ASSERT(it == list.begin());
    ASSERT(*it == 2);
    ASSERT(*(++it) == 1);
  }
  {
    List<int> list;
    list.append(1);
    List<int>::Iterator it = list.insert(list.end(), 2);
    ASSERT(it != list.begin());
    ASSERT(*it == 2);
    ASSERT(*(--it) == 1);
  }

  // insert list
  {
    List<int> list;
    list.append(1);
    list.append(2);
    List<int> otherList;
    otherList.append(10);
    otherList.append(11);
    otherList.append(12);
    List<int>::Iterator i = list.begin();
    ++i;
    ASSERT(*i == 2);
    i = list.insert(i, otherList);
    ASSERT(*i == 10);
    ASSERT(list.size() == 5);
    i = list.begin();
    ASSERT(*i == 1);
    ASSERT(*(++i) == 10);
    ASSERT(*(++i) == 11);
    ASSERT(*(++i) == 12);
    ASSERT(*(++i) == 2);
    ASSERT(++i == list.end());
    List<int> emptyList;
    i = list.begin();
    ++i;
    ASSERT(list.insert(i, emptyList) == i);
    ASSERT(list.size() == 5);
  }

  // prepend list
  {
    List<int> list;
    list.append(1);
    list.append(2);
    List<int> otherList;
    otherList.append(10);
    otherList.append(11);
    otherList.append(12);
    list.prepend(otherList);
    ASSERT(list.size() == 5);
    List<int>::Iterator i = list.begin();
    ASSERT(*i == 10);
    ASSERT(*(++i) == 11);
    ASSERT(*(++i) == 12);
    ASSERT(*(++i) == 1);
    ASSERT(*(++i) == 2);
    ASSERT(++i == list.end());
    List<int> emptyList;
    list.prepend(emptyList);
    ASSERT(list.size() == 5);
  }
  {
    List<int> list;
    List<int> otherList;
    otherList.append(10);
    otherList.append(11);
    otherList.append(12);
    list.prepend(otherList);
    ASSERT(list.size() == 3);
    List<int>::Iterator i = list.begin();
    ASSERT(*i == 10);
    ASSERT(*(++i) == 11);
    ASSERT(*(++i) == 12);
    ASSERT(++i == list.end());
  }

  // append list
  {
    List<int> list;
    list.append(1);
    list.append(2);
    List<int> otherList;
    otherList.append(10);
    otherList.append(11);
    otherList.append(12);
    list.append(otherList);
    ASSERT(list.size() == 5);
    List<int>::Iterator i = list.begin();
    ASSERT(*i == 1);
    ASSERT(*(++i) == 2);
    ASSERT(*(++i) == 10);
    ASSERT(*(++i) == 11);
    ASSERT(*(++i) == 12);
    ASSERT(++i == list.end());
    List<int> emptyList;
    list.prepend(emptyList);
    ASSERT(list.size() == 5);
  }
  {
    List<int> list;
    List<int> otherList;
    otherList.append(10);
    otherList.append(11);
    otherList.append(12);
    list.append(otherList);
    ASSERT(list.size() == 3);
    List<int>::Iterator i = list.begin();
    ASSERT(*i == 10);
    ASSERT(*(++i) == 11);
    ASSERT(*(++i) == 12);
    ASSERT(++i == list.end());
  }
}

int main(int argc, char* argv[])
{
    testList();
    return 0;
}
