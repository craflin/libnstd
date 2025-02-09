
#include <nstd/File.hpp>
#include <nstd/Debug.hpp>
#include <nstd/Time.hpp>
#include <nstd/Error.hpp>

void testFile()
{
  // test open and close
  File::unlink("testfile.file.test");
  ASSERT(!File::exists("testfile.file.test"));
  {
    File file;
    ASSERT(!file.isOpen());
    ASSERT(file.open("testfile.file.test", File::writeFlag));
    ASSERT(file.isOpen());
    file.close();
    ASSERT(!file.isOpen());
    ASSERT(file.open("testfile.file.test", File::writeFlag));
    ASSERT(file.isOpen());
  }

  // test File::time function
  {
    File::Time time;
    ASSERT(File::time("testfile.file.test", time));
    int64 now = Time::time();
    //ASSERT(time.accessTime <= now + 1000 && time.accessTime > now - 10000);
    ASSERT(time.writeTime <= now + 1000 && time.writeTime > now - 10000);
    //ASSERT(time.creationTime <= now + 1000 && time.creationTime > now - 10000);
  }

  // test file exists
  ASSERT(File::exists("testfile.file.test"));
  ASSERT(!File::exists("dkasdlakshkalal.nonexisting.file"));

  // test file write
  char buffer[266];
  char buffer2[300];
  {
    File file;
    ASSERT(file.open("testfile.file.test", File::writeFlag));
    Memory::fill(buffer, 'a', sizeof(buffer));
    ASSERT(file.write(buffer, sizeof(buffer)) == sizeof(buffer));
    Memory::fill(buffer2, 'b', sizeof(buffer2));
    ASSERT(file.write(buffer2, sizeof(buffer2)) == sizeof(buffer2));
    ASSERT(file.size() == sizeof(buffer) + sizeof(buffer2));
  }

  // test file read
  {
    File file;
    ASSERT(file.open("testfile.file.test", File::readFlag));
    ASSERT(file.size() == sizeof(buffer) + sizeof(buffer2));
    char readBuffer[500];
    ASSERT(file.read(readBuffer, sizeof(readBuffer)) == sizeof(readBuffer));
    ASSERT(Memory::compare(readBuffer, buffer, sizeof(buffer)) == 0);
    ASSERT(Memory::compare(readBuffer + sizeof(buffer), buffer2, sizeof(readBuffer) - sizeof(buffer)) == 0);
    char readBuffer2[166];
    ASSERT(file.read(readBuffer2, sizeof(readBuffer2)) == sizeof(buffer) + sizeof(buffer2) - sizeof(readBuffer));
    ASSERT(Memory::compare(readBuffer2, buffer2 + sizeof(buffer2) - (sizeof(buffer) + sizeof(buffer2) - sizeof(readBuffer)), sizeof(buffer) + sizeof(buffer2) - sizeof(readBuffer)) == 0);
    file.close();
  }

  // test file read all
  {
    File file;
    ASSERT(file.open("testfile.file.test", File::readFlag));
    ASSERT(file.size() == sizeof(buffer) + sizeof(buffer2));
    String data;
    ASSERT(file.readAll(data));
    ASSERT(data.length()  == sizeof(buffer) + sizeof(buffer2));
    ASSERT(Memory::compare((const char*)data, buffer, sizeof(buffer)) == 0);
    ASSERT(Memory::compare((const byte*)(const char*)data + sizeof(buffer), buffer2, sizeof(buffer2)) == 0);
  }

  // test unlink
  ASSERT(File::unlink("testfile.file.test"));
  ASSERT(!File::exists("testfile.file.test"));

  // test append mode
  {
    char buf1[10];
    char buf2[20];
    Memory::fill(buf1, 1, sizeof(buf1));
    Memory::fill(buf2, 1, sizeof(buf2));
    {
      File file;
      ASSERT(file.open("testfile.file.test", File::writeFlag));
      ASSERT(file.write(buf1, sizeof(buf1)) == sizeof(buf1));
    }
    {
      File file;
      ASSERT(file.open("testfile.file.test", File::writeFlag | File::appendFlag));
      ASSERT(file.write(buf2, sizeof(buf2)) == sizeof(buf2));
    }
    {
      File file;
      ASSERT(file.open("testfile.file.test", File::readFlag));
      char result[50];
      ASSERT(file.read(result, sizeof(result)) == sizeof(buf1) + sizeof(buf2));
      ASSERT(Memory::compare(result, buf1, sizeof(buf1)) == 0);
      ASSERT(Memory::compare(result + sizeof(buf1), buf2, sizeof(buf2)) == 0);
    }
  }
  ASSERT(File::unlink("testfile.file.test"));

  // test rename function
  {
      File::unlink("testfile.file.test2");
      File file;
      ASSERT(file.open("testfile.file.test", File::writeFlag));
      file.close();
      ASSERT(File::rename("testfile.file.test", "testfile.file.test2"));
      ASSERT(!File::exists("testfile.file.test"));
      ASSERT(!File::rename("testfile.file.test", "testfile.file.test2"));
      ASSERT(file.open("testfile.file.test", File::writeFlag));
      file.close();
      ASSERT(!File::rename("testfile.file.test", "testfile.file.test2"));
      ASSERT(File::rename("testfile.file.test", "testfile.file.test2", false));
      ASSERT(!File::exists("testfile.file.test"));
      File::unlink("testfile.file.test2");
  }

  // test file name functions
  {
    ASSERT(File::getBaseName("c:\\sadasd\\asdas\\test.blah") == "test.blah");
    ASSERT(File::getBaseName("c:\\sadasd\\asdas\\test") == "test");
    ASSERT(File::getBaseName("c:\\sadasd\\asdas\\test.blah", "blah") == "test");
    ASSERT(File::getBaseName("c:\\sadasd\\asdas\\test.blah", ".blah") == "test");
    ASSERT(File::getBaseName("c:\\sadasd\\asdas\\test.blah", ".xy") == "test.blah");
    ASSERT(File::getStem("c:\\sadasd\\asdas\\test.blah") == "test");
    ASSERT(File::getStem("c:\\sadasd\\asdas\\test") == "test");
    ASSERT(File::getStem("c:\\sadasd\\asdas\\test.blah", "blah") == "test");
    ASSERT(File::getStem("c:\\sadasd\\asdas\\test.blah", ".blah") == "test");
    ASSERT(File::getExtension("c:\\sadasd\\asdas\\test.blah") == "blah");
    ASSERT(File::getDirectoryName("c:\\sadasd\\asdas\\test.blah") == "c:\\sadasd\\asdas");
    ASSERT(File::getDirectoryName("asdas/test.blah") == "asdas");

    ASSERT(File::simplifyPath("../../dsadsad/2dsads") == "../../dsadsad/2dsads");
    ASSERT(File::simplifyPath("..\\..\\dsadsad\\2dsads") == "../../dsadsad/2dsads");
    ASSERT(File::simplifyPath(".././../dsadsad/2dsads") == "../../dsadsad/2dsads");
    ASSERT(File::simplifyPath("dsadsad/../2dsads") == "2dsads");
    ASSERT(File::simplifyPath("dsadsad/./../2dsads") == "2dsads");
    ASSERT(File::simplifyPath("dsadsad/.././2dsads") == "2dsads");
    ASSERT(File::simplifyPath("/dsadsad/../2dsads") == "/2dsads");
    ASSERT(File::simplifyPath("/../../aaa/2dsads") == "/../../aaa/2dsads");
    ASSERT(File::simplifyPath("/dsadsad/../2dsads/") == "/2dsads");
    ASSERT(File::simplifyPath("dsadsad\\") == "dsadsad");

    ASSERT(File::isAbsolutePath("/aaa/2dsads"));
    ASSERT(File::isAbsolutePath("\\aaa\\2dsads"));
    ASSERT(File::isAbsolutePath("c:/aaa/2dsads"));
    ASSERT(File::isAbsolutePath("c:\\aaa\\2dsads"));
    ASSERT(!File::isAbsolutePath("..\\aaa\\2dsads"));
    ASSERT(!File::isAbsolutePath("aaa/2dsads"));
  }
}

int main(int argc, char* argv[])
{
    testFile();
    return 0;
}
