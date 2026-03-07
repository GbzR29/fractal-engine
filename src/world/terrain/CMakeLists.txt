# =====================
# Terrain Module
# =====================
add_library(FractalEngine_World_Terrain STATIC)

target_sources(FractalEngine_World_Terrain PRIVATE
    Chunk.cpp
    ChunkMesh.cpp
    HeightMap.cpp
    TerrainGenerator.cpp
)

target_include_directories(FractalEngine_World_Terrain PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/world/terrain
)
target_link_libraries(FractalEngine_World_Terrain PUBLIC
    FractalEngine_Graphics
    FractalEngine_Math
    FractalEngine_World
)


if(MINGW OR UNIX)
    target_compile_options(FractalEngine_World_Terrain PRIVATE -Wall -Wextra)
endif()
