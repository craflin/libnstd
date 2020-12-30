
#include <nstd/HashSet.hpp>
#include <nstd/Debug.hpp>
#include <nstd/String.hpp>
#include <nstd/Math.hpp>

struct TestHashSetDestructor
{
  int num;

  TestHashSetDestructor() : num(0) {}
  TestHashSetDestructor(int num) : num(num) {}
  ~TestHashSetDestructor() {++destructions;}

  operator usize() const {return num;}

  static int destructions;
};

int TestHashSetDestructor::destructions = 0;

void testHashSet()
{
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

  // ??
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

  // with string
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
}

int main(int argc, char* argv[])
{
    testHashSet();
    return 0;
}
