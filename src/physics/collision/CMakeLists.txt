# =====================
# Collision Module
# =====================
add_library(FractalEngine_Physics_Collision STATIC)

target_sources(FractalEngine_Physics_Collision PRIVATE
    AABB.cpp
    BVH.cpp
    CollisionDetection.cpp
    Raycast.cpp
)

target_include_directories(FractalEngine_Physics_Collision PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/physics/collision
)
target_link_libraries(FractalEngine_Physics_Collision PUBLIC
    FractalEngine_Physics
)


if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Physics_Collision PRIVATE -Wall -Wextra)
endif()
