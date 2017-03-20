
#include <nstd/Debug.h>
#include <nstd/RefCount.h>

void testRefCount()
{
  static int count = 0;

  class TestObject : public RefCount::Object
  {
  public:
    TestObject() {++count;}
    ~TestObject() {--count;}
  };

  // test construction and destruction
  {
    RefCount::Ptr<TestObject> ptr = new TestObject;
    ASSERT(count == 1);
  }
  ASSERT(count == 0);

  // test copy constructor
  {
    RefCount::Ptr<TestObject> ptr3;
    RefCount::Ptr<TestObject> ptr = new TestObject;
    ASSERT(ptr != ptr3);
    RefCount::Ptr<TestObject> ptr2(ptr);
    ASSERT(ptr == ptr2);
    ptr3 = ptr2;
    ASSERT(ptr == ptr3);
    ASSERT(count == 1);
  }
  ASSERT(count == 0);

}
