# =====================
# Scene Module
# =====================
add_library(FractalEngine_Scene STATIC)

target_sources(FractalEngine_Scene PRIVATE
    Components.cpp
    Player.cpp
    Scene.cpp
)

target_include_directories(FractalEngine_Scene PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/scene
)
target_link_libraries(FractalEngine_Scene PUBLIC
    FractalEngine_Graphics
    FractalEngine_Input
    FractalEngine_World
)


if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Scene PRIVATE -Wall -Wextra)
endif()
