# =====================
# Input Module
# =====================
add_library(FractalEngine_Input STATIC)

target_sources(FractalEngine_Input PRIVATE
    Gamepad.cpp
    Input.cpp
    InputManager.cpp
    InputMapper.cpp
    Keyboard.cpp
    Mouse.cpp
)

target_include_directories(FractalEngine_Input PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/input
)

if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Input PRIVATE -Wall -Wextra)
endif()
