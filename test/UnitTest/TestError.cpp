
#include <nstd/Error.hpp>
#include <nstd/Debug.hpp>
#include <nstd/File.hpp>

void testError()
{
  // test system error code
  {
    File file;
    ASSERT(!file.open(_T("thisshouldbeanonexisting file")));
    ASSERT(!Error::getErrorString().isEmpty());
  }

  // test user error code
  {
    Error::setErrorString(_T("blah"));
    ASSERT(Error::getErrorString() == _T("blah"));
  }

  // test user error code again
  {
    Error::setErrorString(_T("blah"));
    ASSERT(Error::getErrorString() == _T("blah"));
  }
}

int main(int argc, char* argv[])
{
    testError();
    return 0;
}
