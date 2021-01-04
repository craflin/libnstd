
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
    ../include/nstd/Array.hpp
    ../include/nstd/Atomic.hpp
    ../include/nstd/Base.hpp
    ../include/nstd/Buffer.hpp
    ../include/nstd/Call.hpp
    ../include/nstd/Callback.hpp
    ../include/nstd/Console.hpp
    ../include/nstd/Debug.hpp
    ../include/nstd/Directory.hpp
    ../include/nstd/Error.hpp
    ../include/nstd/File.hpp
    ../include/nstd/Future.hpp
    ../include/nstd/HashMap.hpp
    ../include/nstd/HashSet.hpp
    ../include/nstd/Library.hpp
    ../include/nstd/List.hpp
    ../include/nstd/Log.hpp
    ../include/nstd/Map.hpp
    ../include/nstd/Math.hpp
    ../include/nstd/Memory.hpp
    ../include/nstd/Monitor.hpp
    ../include/nstd/MultiMap.hpp
    ../include/nstd/Mutex.hpp
    ../include/nstd/PoolList.hpp
    ../include/nstd/PoolMap.hpp
    ../include/nstd/Process.hpp
    ../include/nstd/RefCount.hpp
    ../include/nstd/Semaphore.hpp
    ../include/nstd/Signal.hpp
    ../include/nstd/String.hpp
    ../include/nstd/System.hpp
    ../include/nstd/Thread.hpp
    ../include/nstd/Time.hpp
    ../include/nstd/Unicode.hpp
    ../include/nstd/Variant.hpp
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
    if(UNICODE)
        target_compile_definitions(nstd PUBLIC
            UNICODE _UNICODE
        )
    endif()
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
