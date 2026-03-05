# CMakeLists.txt for Input Module
# Subdirectory: src/input

add_library(fractal_engine_input
    OBJECT
    Input.cpp
)

target_include_directories(fractal_engine_input
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/third_party
)
