# =====================
# Ecs Module
# =====================
add_library(FractalEngine_Scene_Ecs STATIC)

target_sources(FractalEngine_Scene_Ecs PRIVATE
    ComponentPool.cpp
    Entity.cpp
    Registry.cpp
    System.cpp
)

target_include_directories(FractalEngine_Scene_Ecs PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/scene/ecs
)

if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Scene_Ecs PRIVATE -Wall -Wextra)
endif()
