# =====================
# Core Module
# =====================
add_library(FractalEngine_Core STATIC)

target_sources(FractalEngine_Core PRIVATE
    Application.cpp
    Engine.cpp
    GameScene.cpp
    Program.cpp
    Time.cpp
    Window.cpp
)

target_include_directories(FractalEngine_Core PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/core
)
target_link_libraries(FractalEngine_Core PUBLIC
    FractalEngine_Graphics
    FractalEngine_Input
)


if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Core PRIVATE -Wall -Wextra)
endif()
