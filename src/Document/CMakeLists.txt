
set(sources
    JSON.cpp
    XML.cpp
    ../../include/nstd/Document/JSON.h
    ../../include/nstd/Document/XML.h
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
