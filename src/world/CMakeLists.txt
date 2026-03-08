# =====================
# World Module
# =====================
add_library(FractalEngine_World STATIC)

target_sources(FractalEngine_World PRIVATE
    Raycast.cpp
    World.cpp
    WorldManager.cpp
    BlockRegistry.cpp
)

target_include_directories(FractalEngine_World PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/world
)
target_link_libraries(FractalEngine_World PUBLIC
    FractalEngine_Graphics
    FractalEngine_Math
)


if(MINGW OR UNIX)
    target_compile_options(FractalEngine_World PRIVATE -Wall -Wextra)
endif()
