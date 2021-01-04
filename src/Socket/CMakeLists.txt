
set(sources
    Server.cpp
    Socket.cpp
    ../../include/nstd/Socket/Server.hpp
    ../../include/nstd/Socket/Socket.hpp
)

add_library(nstdSocket STATIC
    ${sources}
)
add_library(libnstd::Socket ALIAS nstdSocket)

target_link_libraries(nstdSocket PUBLIC
    nstd
)

if(WIN32)
    target_link_libraries(nstdSocket PUBLIC
        ws2_32
    )
endif()
if(MSVC AND UNICODE)
    target_link_libraries(nstdSocket PUBLIC
        ntdll
    )
endif()

source_group("" FILES ${sources})
set_property(TARGET nstdSocket PROPERTY FOLDER "src")
