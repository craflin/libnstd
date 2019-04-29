cmake_minimum_required(VERSION 3.1)

project(libnstd)

find_package(Threads)

add_library(libnstd STATIC
    src/Callback.cpp
    src/Console.cpp
    src/Crypto
    src/Debug.cpp
    src/Directory.cpp
    src/Document
    src/Error.cpp
    src/File.cpp
    src/Future.cpp
    src/Library.cpp
    src/Log.cpp
    src/Math.cpp
    src/Memory.cpp
    src/Monitor.cpp
    src/Mutex.cpp
    src/Process.cpp
    src/Semaphore.cpp
    src/Signal.cpp
    src/Socket
    src/String.cpp
    src/System.cpp
    src/Thread.cpp
    src/Time.cpp
    src/Variant.cpp
    src/Crypto/Sha256.cpp
    src/Document/JSON.cpp
    src/Document/XML.cpp
    src/Socket/Server.cpp
    src/Socket/Socket.cpp
)

target_include_directories(libnstd PUBLIC
    include
)
target_link_libraries(libnstd PUBLIC
    Threads::Threads
)
if(MSVC)
    target_compile_definitions(libnstd PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
    target_link_libraries(libnstd PUBLIC
        ntdll
    )
endif()
if(WIN32)
    target_compile_definitions(libnstd PRIVATE
        _WIN32_WINNT=0x0600
    )
    target_link_libraries(libnstd PUBLIC
        ws2_32
    )
else()
    target_link_libraries(libnstd PUBLIC
        rt
    )
endif()
