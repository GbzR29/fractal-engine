# =====================
# Renderer Module
# =====================
add_library(FractalEngine_Renderer STATIC)

target_sources(FractalEngine_Renderer PRIVATE
    Mesh.cpp
    MeshBuilder.cpp
    RenderGraph.cpp
    RenderPass.cpp
    Renderer.cpp
)

target_include_directories(FractalEngine_Renderer PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/renderer
)
target_link_libraries(FractalEngine_Renderer PUBLIC
    FractalEngine_Graphics
)


if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Renderer PRIVATE -Wall -Wextra)
endif()
