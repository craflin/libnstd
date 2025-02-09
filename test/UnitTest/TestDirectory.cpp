
#include <nstd/Directory.hpp>
#include <nstd/Debug.hpp>
#include <nstd/File.hpp>
#include <nstd/List.hpp>

void testDirectory()
{
  Directory::unlink("testDir", true);

  // test create directory
  ASSERT(!Directory::exists("testDir"));
  ASSERT(Directory::create("testDir"));

  // test searching in an empty directory
  {
    Directory dir;
    ASSERT(dir.open("testDir", "*", false));
    String path;
    bool isDir;
    ASSERT(!dir.read(path, isDir));
  }
  {
    Directory dir;
    ASSERT(dir.open("testDir", String(), false));
    String path;
    bool isDir;
    ASSERT(!dir.read(path, isDir));
  }
  {
    Directory dir;
    ASSERT(dir.open("testDir", "a*", false));
    String path;
    bool isDir;
    ASSERT(!dir.read(path, isDir));
  }

  // create some test files
  List<String> files;
  files.append("ab");
  files.append("ab.t");
  files.append("ab.tx");
  files.append("ab.txt");
  files.append("ab.txtt");
  files.sort();
  for(List<String>::Iterator i = files.begin(), end = files.end(); i != end; ++i)
    ASSERT(File().open(String("testDir/") + *i, File::writeFlag));

  // create some test dirs
  List<String> dirs;
  dirs.append("dirA");
  dirs.append("dirB");
  dirs.sort();
  for(List<String>::Iterator i = dirs.begin(), end = dirs.end(); i != end; ++i)
    ASSERT(Directory::create(String("testDir/") + *i));

  // search without pattern
  {
    Directory dir;
    ASSERT(dir.open("testDir", String(), false));
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
    ASSERT(dir.open("testDir", "*", false));
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
    ASSERT(dir.open("testDir", String(), true));
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
    ASSERT(dir.open("testDir", "*", true));
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
    ASSERT(dir.open("testDir", "*b?t", false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    foundFiles.sort();
    foundDirs.sort();
    ASSERT(foundFiles.size() == 1 && foundFiles.front() == "ab.t");
    ASSERT(foundDirs.isEmpty());
  }

  // search for *b.*, should not find ab
  {
    Directory dir;
    ASSERT(dir.open("testDir", "*b.*", false));
    String path;
    bool isDir;
    List<String> foundFiles;
    List<String> foundDirs;
    while(dir.read(path, isDir))
      (isDir ? foundDirs : foundFiles).append(path);
    foundFiles.sort();
    foundDirs.sort();
    for(List<String>::Iterator i = foundFiles.begin(), end = foundFiles.end(); i != end; ++i)
      ASSERT(*i != "ab");
    ASSERT(foundFiles.size() == 4);
    ASSERT(foundDirs.isEmpty());
  }

  // search for *.txt, should find ab.txt
  {
    Directory dir;
    ASSERT(dir.open("testDir", "*.txt", false));
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
    ASSERT(dir.open("testDir", "a?.*", false));
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
    ASSERT(dir.open("testDir", "ab?.*", false));
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
    ASSERT(Directory::change("testDir"));
#ifdef _WIN32
    ASSERT(Directory::getCurrentDirectory() == currentDir + "\\testDir");
#else
    ASSERT(Directory::getCurrentDirectory() == currentDir + "/testDir");
#endif
    ASSERT(Directory::change(".."));
    ASSERT(Directory::getCurrentDirectory() == currentDir);
  }

  // delete test files
  for(List<String>::Iterator i = files.begin(), end = files.end(); i != end; ++i)
    ASSERT(File::unlink(String("testDir/") + *i));

  // delete test dirs
  for(List<String>::Iterator i = dirs.begin(), end = dirs.end(); i != end; ++i)
    ASSERT(Directory::unlink(String("testDir/") + *i));

  // delete test dir
  ASSERT(Directory::create(String("testDir/x/y")));
  ASSERT(Directory::create(String("testDir/x2/y")));
  for(List<String>::Iterator i = files.begin(), end = files.end(); i != end; ++i)
    ASSERT(File().open(String("testDir/") + *i, File::writeFlag));
  ASSERT(Directory::unlink(String("testDir"), true));
  ASSERT(!File::exists(String("testDir")));

  // purge test dir
  ASSERT(Directory::create(String("testDir/x/y1")));
  ASSERT(Directory::create(String("testDir/x/y2")));
  for(List<String>::Iterator i = files.begin(), end = files.end(); i != end; ++i)
    ASSERT(File().open(String("testDir/x/") + *i, File::writeFlag));
  ASSERT(Directory::purge(String("testDir/x"), true));
  ASSERT(!File::exists(String("testDir")));

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
