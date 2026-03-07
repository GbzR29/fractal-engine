# =====================
# Graphics Module
# =====================
add_library(FractalEngine_Graphics STATIC)

target_sources(FractalEngine_Graphics PRIVATE
    Camera.cpp
    Framebuffer.cpp
    Material.cpp
    Shader.cpp
    ShaderProgram.cpp
    Texture.cpp
    TextureLoader.cpp
)

target_include_directories(FractalEngine_Graphics PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/graphics
)

if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Graphics PRIVATE -Wall -Wextra)
endif()
