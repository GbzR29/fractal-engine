# CMakeLists.txt for Renderer Module
# Subdirectory: src/renderer

add_library(fractal_engine_renderer
    OBJECT
    Renderer.cpp
    Mesh.cpp
)

target_include_directories(fractal_engine_renderer
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/third_party
)

target_link_libraries(fractal_engine_renderer
    PUBLIC
    opengl32
)
