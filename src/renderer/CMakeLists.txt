# src/renderer/CMakeLists.txt

target_sources(FractalEngineLib PRIVATE
    Mesh.cpp
    MeshBuilder.cpp
    RenderGraph.cpp
    RenderPass.cpp

    # Backend OpenGL (sempre ativo)
    backends/opengl/OpenGLBuffer.cpp
    backends/opengl/OpenGLRenderer.cpp
    backends/opengl/OpenGLShader.cpp
    backends/opengl/OpenGLTexture.cpp
)

# Backend Vulkan (opcional)
if (FE_ENABLE_VULKAN)
    file(GLOB VULKAN_SOURCES "backends/vulkan/*.cpp")
    target_sources(FractalEngineLib PRIVATE ${VULKAN_SOURCES})
endif()