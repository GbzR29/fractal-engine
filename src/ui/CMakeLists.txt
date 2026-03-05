# CMakeLists.txt for UI Module
# Subdirectory: src/ui

add_library(fractal_engine_ui
    OBJECT
    core/UIElement.cpp
    core/UIEventSystem.cpp
    core/UIManager.cpp
    core/UIREnderer.cpp
)

target_include_directories(fractal_engine_ui
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/third_party
)

target_link_libraries(fractal_engine_ui
    PUBLIC
    opengl32
)
