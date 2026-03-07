# =====================
# Dynamics Module
# =====================
add_library(FractalEngine_Physics_Dynamics STATIC)

target_sources(FractalEngine_Physics_Dynamics PRIVATE
    Collider.cpp
    Constraint.cpp
    Rigidbody.cpp
)

target_include_directories(FractalEngine_Physics_Dynamics PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/physics/dynamics
)
target_link_libraries(FractalEngine_Physics_Dynamics PUBLIC
    FractalEngine_Physics
)


if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Physics_Dynamics PRIVATE -Wall -Wextra)
endif()
