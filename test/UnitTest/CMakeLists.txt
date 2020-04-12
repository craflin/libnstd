
set(sources
    Main.cpp
    TestAtomic.cpp
    TestBuffer.cpp
    TestCallback.cpp
    TestConsole.cpp
    TestDebug.cpp
    TestDirectory.cpp
    TestError.cpp
    TestFile.cpp
    TestFuture.cpp
    TestHashMap.cpp
    TestHashSet.cpp
    TestJSON.cpp
    TestList.cpp
    TestMap.cpp
    TestMemory.cpp
    TestMonitor.cpp
    TestMultiMap.cpp
    TestMutex.cpp
    TestPoolList.cpp
    TestPoolMap.cpp
    TestProcess.cpp
    TestRefCount.cpp
    TestSemaphore.cpp
    TestServer.cpp
    TestSha256.cpp
    TestSignal.cpp
    TestSocket.cpp
    TestString.cpp
    TestThread.cpp
    TestTime.cpp
    TestUnicode.cpp
    TestVariant.cpp
    TestXML.cpp
)
add_executable(UnitTest ${sources})
target_link_libraries(UnitTest PRIVATE libnstd::Core libnstd::Socket libnstd::Document libnstd::Crypto)
set_property(TARGET UnitTest PROPERTY FOLDER "test")
source_group("" FILES ${sources})

add_test(NAME UnitTest COMMAND UnitTest)


