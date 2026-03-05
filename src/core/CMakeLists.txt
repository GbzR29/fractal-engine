# CMakeLists.txt for Core Module
# Subdirectory: src/core

add_library(fractal_engine_core
    OBJECT
    Program.cpp
    Program.h
    Application.cpp
    Time.cpp
    Window.cpp
)

target_include_directories(fractal_engine_core
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/third_party
)

target_link_libraries(fractal_engine_core
    PUBLIC
    glfw3dll
    glfw3
    opengl32
)
