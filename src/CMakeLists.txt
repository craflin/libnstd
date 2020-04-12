
find_package(Threads)

set(sources
    Callback.cpp
    Console.cpp
    Debug.cpp
    Directory.cpp
    Error.cpp
    File.cpp
    Future.cpp
    Library.cpp
    Log.cpp
    Math.cpp
    Memory.cpp
    Monitor.cpp
    Mutex.cpp
    Process.cpp
    Semaphore.cpp
    Signal.cpp
    String.cpp
    System.cpp
    Thread.cpp
    Time.cpp
    Variant.cpp
    ../include/nstd/Array.h
    ../include/nstd/Atomic.h
    ../include/nstd/Base.h
    ../include/nstd/Buffer.h
    ../include/nstd/Call.h
    ../include/nstd/Callback.h
    ../include/nstd/Console.h
    ../include/nstd/Debug.h
    ../include/nstd/Directory.h
    ../include/nstd/Error.h
    ../include/nstd/File.h
    ../include/nstd/Future.h
    ../include/nstd/HashMap.h
    ../include/nstd/HashSet.h
    ../include/nstd/Library.h
    ../include/nstd/List.h
    ../include/nstd/Log.h
    ../include/nstd/Map.h
    ../include/nstd/Math.h
    ../include/nstd/Memory.h
    ../include/nstd/Monitor.h
    ../include/nstd/MultiMap.h
    ../include/nstd/Mutex.h
    ../include/nstd/PoolList.h
    ../include/nstd/PoolMap.h
    ../include/nstd/Process.h
    ../include/nstd/RefCount.h
    ../include/nstd/Semaphore.h
    ../include/nstd/Signal.h
    ../include/nstd/String.h
    ../include/nstd/System.h
    ../include/nstd/Thread.h
    ../include/nstd/Time.h
    ../include/nstd/Unicode.h
    ../include/nstd/Variant.h
)

add_library(nstd STATIC
    ${sources}
)
add_library(libnstd::Core ALIAS nstd)

target_include_directories(nstd PUBLIC
    ../include
)
target_link_libraries(nstd PUBLIC
    Threads::Threads
)
if(MSVC)
    target_compile_definitions(nstd PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
endif()
if(WIN32)
    target_compile_definitions(nstd PRIVATE
        _WIN32_WINNT=0x0600
    )
else()
    target_link_libraries(nstd PUBLIC
        rt
    )
endif()

set_property(TARGET nstd PROPERTY FOLDER "src")
source_group("" FILES ${sources})

add_subdirectory(Crypto)
add_subdirectory(Document)
add_subdirectory(Socket)
