# CMakeLists.txt for Graphics Module
# Subdirectory: src/graphics

add_library(fractal_engine_graphics
    OBJECT
    Camera.cpp
    Shader.cpp
    Texture.cpp
    TextureLoader.cpp
)

target_include_directories(fractal_engine_graphics
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/third_party
)

target_link_libraries(fractal_engine_graphics
    PUBLIC
    opengl32
)
