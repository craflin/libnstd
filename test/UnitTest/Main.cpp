
#include <nstd/Console.hpp>
#include <nstd/Memory.hpp>
#include <nstd/Debug.hpp>
#include <nstd/Mutex.hpp>
#include <nstd/String.hpp>
#include <nstd/HashSet.hpp>
#include <nstd/List.hpp>
#include <nstd/HashMap.hpp>
#include <nstd/Thread.hpp>
#include <nstd/Time.hpp>
#include <nstd/Error.hpp>
#include <nstd/Variant.hpp>
#include <nstd/Math.hpp>

void testAtomic();
void testConsole();
void testDebug();
void testProcess();
void testMonitor();
void testSignal();
void testUnicode();
void testHashSet();
void testBuffer();
void testThread();
void testSempahore();
void testDirectory();
void testMutex();
void testPoolList();
void testPoolMap();
void testSocket();
void testServer();
void testMultiMap();
void testError();
void testString();
void testMemory();
void testVariant();
void testFile();
void testXml();
void testJson();
void testCallback();
void testTime();
void testMap();
void testSha256();
void testMap();
void testRefCount(); 
void testList();
void testHashMap();
void testFuture();
void testArray();
void testArrayString();
void testNewDelete();

int main(int argc, char* argv[])
{
  Console::printf(_T("%s\n"), _T("Testing..."));

  testMemory();
  testSha256();
  testSocket();
  testServer();
  testSignal();
  testMonitor();
  testUnicode();
  testBuffer();
  testThread();
  testSempahore();
  testMutex();
  //testConsole();
  //testDebug();
  testAtomic();
  testString();
  testHashSet();
  testList();
  testHashMap();
  testNewDelete();
  testFile();
  testArray();
  testArrayString();
  testDirectory();
  testTime();
  testProcess();
  testMap();
  testPoolList();
  testPoolMap();
  testMultiMap();
  testError();
  testVariant();
  testXml();
  testJson();
  testRefCount();
  testCallback();
  testFuture();

  Console::printf(_T("%s\n"), _T("done"));

  return 0;
}
