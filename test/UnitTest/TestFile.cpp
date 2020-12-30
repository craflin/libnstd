
#include <nstd/File.hpp>
#include <nstd/Debug.hpp>
#include <nstd/Time.hpp>
#include <nstd/Error.hpp>

void testFile()
{
  // test open and close
  File::unlink(_T("testfile.file.test"));
  ASSERT(!File::exists(_T("testfile.file.test")));
  {
    File file;
    ASSERT(!file.isOpen());
    ASSERT(file.open(_T("testfile.file.test"), File::writeFlag));
    ASSERT(file.isOpen());
    file.close();
    ASSERT(!file.isOpen());
    ASSERT(file.open(_T("testfile.file.test"), File::writeFlag));
    ASSERT(file.isOpen());
  }

  // test File::time function
  {
    File::Time time;
    ASSERT(File::time(_T("testfile.file.test"), time));
    int64 now = Time::time();
    //ASSERT(time.accessTime <= now + 1000 && time.accessTime > now - 10000);
    ASSERT(time.writeTime <= now + 1000 && time.writeTime > now - 10000);
    //ASSERT(time.creationTime <= now + 1000 && time.creationTime > now - 10000);
  }

  // test file exists
  ASSERT(File::exists(_T("testfile.file.test")));
  ASSERT(!File::exists(_T("dkasdlakshkalal.nonexisting.file")));

  // test file write
  char buffer[266];
  char buffer2[300];
  {
    File file;
    ASSERT(file.open(_T("testfile.file.test"), File::writeFlag));
    Memory::fill(buffer, 'a', sizeof(buffer));
    ASSERT(file.write(buffer, sizeof(buffer)) == sizeof(buffer));
    Memory::fill(buffer2, 'b', sizeof(buffer2));
    ASSERT(file.write(buffer2, sizeof(buffer2)) == sizeof(buffer2));
    ASSERT(file.size() == sizeof(buffer) + sizeof(buffer2));
  }

  // test file read
  {
    File file;
    ASSERT(file.open(_T("testfile.file.test"), File::readFlag));
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
    ASSERT(file.open(_T("testfile.file.test"), File::readFlag));
    ASSERT(file.size() == sizeof(buffer) + sizeof(buffer2));
    String data;
    ASSERT(file.readAll(data));
    ASSERT(data.length() * sizeof(tchar) == sizeof(buffer) + sizeof(buffer2));
    ASSERT(Memory::compare((const tchar*)data, buffer, sizeof(buffer)) == 0);
    ASSERT(Memory::compare((const byte*)(const tchar*)data + sizeof(buffer), buffer2, sizeof(buffer2)) == 0);
  }

  // test unlink
  ASSERT(File::unlink(_T("testfile.file.test")));
  ASSERT(!File::exists(_T("testfile.file.test")));

  // test append mode
  {
    char buf1[10];
    char buf2[20];
    Memory::fill(buf1, 1, sizeof(buf1));
    Memory::fill(buf2, 1, sizeof(buf2));
    {
      File file;
      ASSERT(file.open(_T("testfile.file.test"), File::writeFlag));
      ASSERT(file.write(buf1, sizeof(buf1)) == sizeof(buf1));
    }
    {
      File file;
      ASSERT(file.open(_T("testfile.file.test"), File::writeFlag | File::appendFlag));
      ASSERT(file.write(buf2, sizeof(buf2)) == sizeof(buf2));
    }
    {
      File file;
      ASSERT(file.open(_T("testfile.file.test"), File::readFlag));
      char result[50];
      ASSERT(file.read(result, sizeof(result)) == sizeof(buf1) + sizeof(buf2));
      ASSERT(Memory::compare(result, buf1, sizeof(buf1)) == 0);
      ASSERT(Memory::compare(result + sizeof(buf1), buf2, sizeof(buf2)) == 0);
    }
  }
  ASSERT(File::unlink(_T("testfile.file.test")));

  // test rename function
  {
      File::unlink(_T("testfile.file.test2"));
      File file;
      ASSERT(file.open(_T("testfile.file.test"), File::writeFlag));
      file.close();
      ASSERT(File::rename(_T("testfile.file.test"), _T("testfile.file.test2")));
      ASSERT(!File::exists(_T("testfile.file.test")));
      ASSERT(!File::rename(_T("testfile.file.test"), _T("testfile.file.test2")));
      ASSERT(file.open(_T("testfile.file.test"), File::writeFlag));
      file.close();
      ASSERT(!File::rename(_T("testfile.file.test"), _T("testfile.file.test2")));
      ASSERT(File::rename(_T("testfile.file.test"), _T("testfile.file.test2"), false));
      ASSERT(!File::exists(_T("testfile.file.test")));
      File::unlink(_T("testfile.file.test2"));
  }

  // test file name functions
  {
    ASSERT(File::basename(_T("c:\\sadasd\\asdas\\test.blah")) == _T("test.blah"));
    ASSERT(File::basename(_T("c:\\sadasd\\asdas\\test")) == _T("test"));
    ASSERT(File::basename(_T("c:\\sadasd\\asdas\\test.blah"), _T("blah")) == _T("test"));
    ASSERT(File::basename(_T("c:\\sadasd\\asdas\\test.blah"), _T(".blah")) == _T("test"));
    ASSERT(File::extension(_T("c:\\sadasd\\asdas\\test.blah")) == _T("blah"));
    ASSERT(File::dirname(_T("c:\\sadasd\\asdas\\test.blah")) == _T("c:\\sadasd\\asdas"));
    ASSERT(File::dirname(_T("asdas/test.blah")) == _T("asdas"));

    ASSERT(File::simplifyPath(_T("../../dsadsad/2dsads")) == _T("../../dsadsad/2dsads"));
    ASSERT(File::simplifyPath(_T("..\\..\\dsadsad\\2dsads")) == _T("../../dsadsad/2dsads"));
    ASSERT(File::simplifyPath(_T(".././../dsadsad/2dsads")) == _T("../../dsadsad/2dsads"));
    ASSERT(File::simplifyPath(_T("dsadsad/../2dsads")) == _T("2dsads"));
    ASSERT(File::simplifyPath(_T("dsadsad/./../2dsads")) == _T("2dsads"));
    ASSERT(File::simplifyPath(_T("dsadsad/.././2dsads")) == _T("2dsads"));
    ASSERT(File::simplifyPath(_T("/dsadsad/../2dsads")) == _T("/2dsads"));
    ASSERT(File::simplifyPath(_T("/../../aaa/2dsads")) == _T("/../../aaa/2dsads"));
    ASSERT(File::simplifyPath(_T("/dsadsad/../2dsads/")) == _T("/2dsads"));
    ASSERT(File::simplifyPath(_T("dsadsad\\")) == _T("dsadsad"));

    ASSERT(File::isAbsolutePath(_T("/aaa/2dsads")));
    ASSERT(File::isAbsolutePath(_T("\\aaa\\2dsads")));
    ASSERT(File::isAbsolutePath(_T("c:/aaa/2dsads")));
    ASSERT(File::isAbsolutePath(_T("c:\\aaa\\2dsads")));
    ASSERT(!File::isAbsolutePath(_T("..\\aaa\\2dsads")));
    ASSERT(!File::isAbsolutePath(_T("aaa/2dsads")));
  }
}

int main(int argc, char* argv[])
{
    testFile();
    return 0;
}
