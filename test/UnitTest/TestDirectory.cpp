
#include <nstd/Directory.hpp>
#include <nstd/Debug.hpp>
#include <nstd/File.hpp>
#include <nstd/List.hpp>

void testDirectory()
{
  Directory::unlink(_T("testDir"), true);

  // test create directory
  ASSERT(!Directory::exists(_T("testDir")));
  ASSERT(Directory::create(_T("testDir")));

  // test searching in an empty directory
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("*"), false));
    String path;
    bool isDir;
    ASSERT(!dir.read(path, isDir));
  }
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), String(), false));
    String path;
    bool isDir;
    ASSERT(!dir.read(path, isDir));
  }
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("a*"), false));
    String path;
    bool isDir;
    ASSERT(!dir.read(path, isDir));
  }

  // create some test files
  List<String> files;
  files.append(_T("ab"));
  files.append(_T("ab.t"));
  files.append(_T("ab.tx"));
  files.append(_T("ab.txt"));
  files.append(_T("ab.txtt"));
  files.sort();
  for(List<String>::Iterator i = files.begin(), end = files.end(); i != end; ++i)
    ASSERT(File().open(String(_T("testDir/")) + *i, File::writeFlag));

  // create some test dirs
  List<String> dirs;
  dirs.append(_T("dirA"));
  dirs.append(_T("dirB"));
  dirs.sort();
  for(List<String>::Iterator i = dirs.begin(), end = dirs.end(); i != end; ++i)
    ASSERT(Directory::create(String(_T("testDir/")) + *i));

  // search without pattern
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), String(), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    foundFiles.sort();
    foundDirs.sort();
    ASSERT(foundFiles == files);
    ASSERT(foundDirs == dirs);
  }

  // search for '*'
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("*"), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    foundFiles.sort();
    foundDirs.sort();
    ASSERT(foundFiles == files);
    ASSERT(foundDirs == dirs);
  }

  // search without pattern (dirs only)
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), String(), true));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    foundFiles.sort();
    foundDirs.sort();
    ASSERT(foundFiles.isEmpty());
    ASSERT(foundDirs == dirs);
  }

  // search for '*' (dirs only)
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("*"), true));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    foundFiles.sort();
    foundDirs.sort();
    ASSERT(foundFiles.isEmpty());
    ASSERT(foundDirs == dirs);
  }

  // seach for *a?t, should find ab.t
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("*b?t"), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    foundFiles.sort();
    foundDirs.sort();
    ASSERT(foundFiles.size() == 1 && foundFiles.front() == _T("ab.t"));
    ASSERT(foundDirs.isEmpty());
  }

  // search for *b.*, should not find ab
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("*b.*"), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    foundFiles.sort();
    foundDirs.sort();
    for(List<String>::Iterator i = foundFiles.begin(), end = foundFiles.end(); i != end; ++i)
      ASSERT(*i != _T("ab"));
    ASSERT(foundFiles.size() == 4);
    ASSERT(foundDirs.isEmpty());
  }

  // search for *.txt, should find ab.txt
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("*.txt"), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    foundFiles.sort();
    foundDirs.sort();
    ASSERT(foundFiles.size() == 1);
    ASSERT(foundDirs.isEmpty());
  }

  // search for a?.*
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("a?.*"), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    foundFiles.sort();
    foundDirs.sort();
    ASSERT(foundFiles.size() == 4);
    ASSERT(foundDirs.isEmpty());
  }

  // search for ab?.*
  {
    Directory dir;
    ASSERT(dir.open(_T("testDir"), _T("ab?.*"), false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    foundFiles.sort();
    foundDirs.sort();
    ASSERT(foundFiles.isEmpty());
    ASSERT(foundDirs.isEmpty());
  }
  
  // test getCurrentDir and change
  {
    String currentDir = Directory::getCurrentDirectory();
    ASSERT(!currentDir.isEmpty());
    ASSERT(Directory::change(_T("testDir")));
#ifdef _WIN32
    ASSERT(Directory::getCurrentDirectory() == currentDir + _T("\\testDir"));
#else
    ASSERT(Directory::getCurrentDirectory() == currentDir + _T("/testDir"));
#endif
    ASSERT(Directory::change(_T("..")));
    ASSERT(Directory::getCurrentDirectory() == currentDir);
  }

  // delete test files
  for(List<String>::Iterator i = files.begin(), end = files.end(); i != end; ++i)
    ASSERT(File::unlink(String(_T("testDir/")) + *i));

  // delete test dirs
  for(List<String>::Iterator i = dirs.begin(), end = dirs.end(); i != end; ++i)
    ASSERT(Directory::unlink(String(_T("testDir/")) + *i));

  // delete test dir
  ASSERT(Directory::create(String(_T("testDir/x/y"))));
  ASSERT(Directory::create(String(_T("testDir/x2/y"))));
  for(List<String>::Iterator i = files.begin(), end = files.end(); i != end; ++i)
    ASSERT(File().open(String(_T("testDir/")) + *i, File::writeFlag));
  ASSERT(Directory::unlink(String(_T("testDir")), true));
  ASSERT(!File::exists(String(_T("testDir"))));

  // purge test dir
  ASSERT(Directory::create(String(_T("testDir/x/y1"))));
  ASSERT(Directory::create(String(_T("testDir/x/y2"))));
  for(List<String>::Iterator i = files.begin(), end = files.end(); i != end; ++i)
    ASSERT(File().open(String(_T("testDir/x/")) + *i, File::writeFlag));
  ASSERT(Directory::purge(String(_T("testDir/x")), true));
  ASSERT(!File::exists(String(_T("testDir"))));

  // test get temp
  {
    String temp = Directory::getTempDirectory();
    ASSERT(Directory::exists(temp));
  }
}

int main(int argc, char* argv[])
{
    testDirectory();
    return 0;
}
