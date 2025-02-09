
#include <nstd/Error.hpp>
#include <nstd/Debug.hpp>
#include <nstd/File.hpp>

void testError()
{
  // test system error code
  {
    File file;
    ASSERT(!file.open("thisshouldbeanonexisting file"));
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

int main(int argc, char* argv[])
{
    testError();
    return 0;
}
