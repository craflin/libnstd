
#include <nstd/Variant.hpp>
#include <nstd/Debug.hpp>

void testVariant()
{
  // test default constructor
  {
    Variant var;
    ASSERT(var.isNull());
    ASSERT(var.toBool() == false);
  }

  // test self assign
  {
    Variant var(String(_T("hallo")));
    ASSERT(var.toString() == _T("hallo"));
    var = var;
    ASSERT(var.toString() == _T("hallo"));

    Variant var2(123);
    ASSERT(var2.toInt() == 123);
    var2 = var2;
    ASSERT(var2.toInt() == 123);
  }

  // test boolean constructor
  {
    Variant var(true);
    ASSERT(var.toBool() == true);
  }

  // test map constructor
  {
    HashMap<String, Variant> map;
    map.append(_T("dasd"), Variant(String(_T("yes"))));
    Variant var(map);
    ASSERT(((const Variant&)var).toMap().find(_T("dasd"))->toString() == _T("yes"));
    ASSERT(var.toMap().find(_T("dasd"))->toString() == _T("yes"));
  }

  // test copy construction of null variant
  {
    Variant null;
    ASSERT(null.isNull());
    Variant copy(null);
    ASSERT(copy.isNull());
  }

  // test list detaching
  {
    Variant var1;
    var1.toList().append(123);
    Variant var2(var1);
    List<Variant>& var2list = var2.toList();
    ASSERT(var2list.size() == 1);
    var2list.clear();
    ASSERT(((const Variant&)var1).toList().size() == 1);
  }
}

int main(int argc, char* argv[])
{
  testVariant();
  return 0;
}
