
set(sources
    Server.cpp
    Socket.cpp
    ../../include/nstd/Socket/Server.h
    ../../include/nstd/Socket/Socket.h
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

source_group("" FILES ${sources})
set_property(TARGET nstdSocket PROPERTY FOLDER "src")
