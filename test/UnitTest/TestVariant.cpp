
#include <nstd/Debug.h>
#include <nstd/Variant.h>

void_t testVariant()
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
    map.append("dasd", Variant(String("yes")));
    Variant var(map);
    ASSERT(((const Variant&)var).toMap().find("dasd")->toString() == "yes");
    ASSERT(var.toMap().find("dasd")->toString() == "yes");
  }

}
