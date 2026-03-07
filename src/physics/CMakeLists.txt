# =====================
# Physics Module
# =====================
add_library(FractalEngine_Physics STATIC)

target_sources(FractalEngine_Physics PRIVATE
    PhysicsEngine.cpp
)

target_include_directories(FractalEngine_Physics PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/physics
)

if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Physics PRIVATE -Wall -Wextra)
endif()
