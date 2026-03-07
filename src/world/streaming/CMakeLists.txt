# =====================
# Streaming Module
# =====================
add_library(FractalEngine_World_Streaming STATIC)

target_sources(FractalEngine_World_Streaming PRIVATE
    AsyncLoader.cpp
    ChunkStreamer.cpp
)

target_include_directories(FractalEngine_World_Streaming PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/world/streaming
)
target_link_libraries(FractalEngine_World_Streaming PUBLIC
    FractalEngine_World
)


if(MINGW OR UNIX)
    target_compile_options(FractalEngine_World_Streaming PRIVATE -Wall -Wextra)
endif()
