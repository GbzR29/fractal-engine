# =====================
# Renderer_Backends/Vulkan Submodule
# =====================
add_library(FractalEngine_Renderer_Backends_Vulkan STATIC)

target_sources(FractalEngine_Renderer_Backends_Vulkan PRIVATE
    VulkanRenderer.cpp
    VulkanDevice.cpp
    VulkanSwapchain.cpp
)

target_include_directories(FractalEngine_Renderer_Backends_Vulkan PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/renderer_backends/vulkan
)
target_link_libraries(FractalEngine_Renderer_Backends_Vulkan PUBLIC
    FractalEngine_Renderer
)


if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Renderer_Backends_Vulkan PRIVATE -Wall -Wextra)
endif()
