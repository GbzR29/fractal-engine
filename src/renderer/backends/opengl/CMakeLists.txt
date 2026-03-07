# =====================
# Renderer_Backends/Opengl Submodule
# =====================
add_library(FractalEngine_Renderer_Backends_Opengl STATIC)

target_sources(FractalEngine_Renderer_Backends_Opengl PRIVATE
    OpenGLRenderer.cpp
    OpenGLBuffer.cpp
    OpenGLTexture.cpp
    OpenGLShader.cpp
)

target_include_directories(FractalEngine_Renderer_Backends_Opengl PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/renderer_backends/opengl
)
target_link_libraries(FractalEngine_Renderer_Backends_Opengl PUBLIC
    FractalEngine_Renderer
    FractalEngine_Graphics
)


if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Renderer_Backends_Opengl PRIVATE -Wall -Wextra)
endif()
