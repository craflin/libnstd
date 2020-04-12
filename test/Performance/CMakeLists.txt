
set(sources
    FutureNStd.cpp
    FutureStd.cpp
    HashMapNStd.cpp
    HashMapStd.cpp
    MapNStd.cpp
    MapStd.cpp
    Performance.cpp
    StringNStd.cpp
    StringStd.cpp
)
add_executable(Performance ${sources})
target_link_libraries(Performance PRIVATE libnstd::Core)
set_property(TARGET Performance PROPERTY FOLDER "test")
source_group("" FILES ${sources})
