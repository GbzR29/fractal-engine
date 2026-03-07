# =====================
# FractalEngine Main Library
# =====================

# Add all module subdirectories
add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(renderer)
add_subdirectory(renderer/backends/opengl)
add_subdirectory(renderer/backends/vulkan)
add_subdirectory(scene)
add_subdirectory(scene/ecs)
add_subdirectory(physics)
add_subdirectory(physics/collision)
add_subdirectory(physics/dynamics)
add_subdirectory(input)
add_subdirectory(world)
add_subdirectory(world/terrain)
add_subdirectory(world/streaming)
add_subdirectory(audio)
add_subdirectory(ui)
add_subdirectory(scripting)
add_subdirectory(math)
add_subdirectory(utils)
add_subdirectory(platform)
add_subdirectory(platform/win32)
add_subdirectory(platform/linux)
add_subdirectory(platform/macos)

# =====================
# Main Executable
# =====================
add_executable(FractalEngine_App 
    main.cpp
)

# Link main executable with all modules
target_link_libraries(FractalEngine_App PRIVATE
    FractalEngine_Core
    FractalEngine_Graphics
    FractalEngine_Renderer
    FractalEngine_Renderer_Backends_Opengl
    FractalEngine_Renderer_Backends_Vulkan
    FractalEngine_Scene
    FractalEngine_Scene_Ecs
    FractalEngine_Physics
    FractalEngine_Physics_Collision
    FractalEngine_Physics_Dynamics
    FractalEngine_Input
    FractalEngine_World
    FractalEngine_World_Terrain
    FractalEngine_World_Streaming
    FractalEngine_Audio
    FractalEngine_Ui
    FractalEngine_Scripting
    FractalEngine_Math
    FractalEngine_Utils
    FractalEngine_Platform
    FractalEngine_Platform_Win32
    FractalEngine_Platform_Linux
    FractalEngine_Platform_Macos
    glad
)

if(MINGW)
    target_link_libraries(FractalEngine_App PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/libglfw3.a
        ${CMAKE_SOURCE_DIR}/lib/libSDL3.dll.a
        ${CMAKE_SOURCE_DIR}/lib/liblua55.a
        opengl32
        gdi32
        winmm
        imm32
        -Wl,--as-needed
    )
    
    # Add compiler flags
    target_compile_options(FractalEngine_App PRIVATE -Wall -Wextra -g)
    target_link_options(FractalEngine_App PRIVATE -fcommon)
endif()

# =====================
# Runtime assets
# =====================
# Copy DLLs if they exist
if(EXISTS "${CMAKE_SOURCE_DIR}/bin")
    file(GLOB RUNTIME_DLLS "${CMAKE_SOURCE_DIR}/bin/*.dll")
    if(RUNTIME_DLLS)
        add_custom_command(TARGET FractalEngine_App POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${RUNTIME_DLLS}
            $<TARGET_FILE_DIR:FractalEngine_App>
            COMMENT "Copying runtime DLLs..."
        )
    endif()
endif()

# Copy shaders and assets
add_custom_command(TARGET FractalEngine_App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets/shaders
    $<TARGET_FILE_DIR:FractalEngine_App>/shaders
    COMMENT "Copying shaders..."
)

add_custom_command(TARGET FractalEngine_App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:FractalEngine_App>/assets
    COMMENT "Copying assets..."
)
