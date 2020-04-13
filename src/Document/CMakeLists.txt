
set(sources
    Json.cpp
    Xml.cpp
    ../../include/nstd/Document/Json.hpp
    ../../include/nstd/Document/Xml.hpp
)

add_library(nstdDocument STATIC
    ${sources}
)
add_library(libnstd::Document ALIAS nstdDocument)

target_link_libraries(nstdDocument PUBLIC
    nstd
)

source_group("" FILES ${sources})
set_property(TARGET nstdDocument PROPERTY FOLDER "src")
