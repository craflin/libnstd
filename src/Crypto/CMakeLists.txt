
set(sources
    Sha256.cpp
    ../../include/nstd/Crypto/Sha256.h
)

add_library(nstdCrypto STATIC
    ${sources}
)
add_library(libnstd::Crypto ALIAS nstdCrypto)

target_link_libraries(nstdCrypto PUBLIC
    nstd
)

source_group("" FILES ${sources})
