
#include <nstd/Debug.h>
#include <nstd/Variant.h>

void testVariant()
{
  // test default constructor
  {
    Variant var;
    ASSERT(var.isNull());
    ASSERT(var.toBool() == false);
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
}
