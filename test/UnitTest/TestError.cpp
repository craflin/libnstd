
#include <nstd/Debug.h>
#include <nstd/Error.h>
#include <nstd/File.h>

void_t testError()
{
  // test system error code
  {
    File file;
    ASSERT(!file.open(_T("thisshouldbeanonexisting file")));
    ASSERT(!Error::getErrorString().isEmpty());
  }

  // test user error code
  {
    Error::setErrorString("blah");
    ASSERT(Error::getErrorString() == "blah");
  }

  // test user error code again
  {
    Error::setErrorString("blah");
    ASSERT(Error::getErrorString() == "blah");
  }
}
