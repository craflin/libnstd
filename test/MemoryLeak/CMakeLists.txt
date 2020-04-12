
set(sources
    Main.cpp
)
add_executable(MemoryLeak ${sources})
target_link_libraries(MemoryLeak PRIVATE libnstd::Core)
set_property(TARGET MemoryLeak PROPERTY FOLDER "test")
source_group("" FILES ${sources})
