# CMakeLists.txt for World Module
# Subdirectory: src/world

add_library(fractal_engine_world
    OBJECT
    Chunk.cpp
    World.cpp
)

target_include_directories(fractal_engine_world
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/third_party
)

target_link_libraries(fractal_engine_world
    PUBLIC
    opengl32
)
