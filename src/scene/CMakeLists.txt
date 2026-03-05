# CMakeLists.txt for Scene Module
# Subdirectory: src/scene

add_library(fractal_engine_scene
    OBJECT
    Player.cpp
    Components.cpp
    Entity.cpp
    Scene.cpp
)

target_include_directories(fractal_engine_scene
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/third_party
)
