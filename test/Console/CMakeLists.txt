
set(sources
    Main.cpp
)
add_executable(Console ${sources})
target_link_libraries(Console PRIVATE libnstd::Core)
set_property(TARGET Console PROPERTY FOLDER "test")
source_group("" FILES ${sources})
