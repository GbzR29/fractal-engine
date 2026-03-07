# =====================
# Math Module
# =====================
add_library(FractalEngine_Math STATIC)

target_sources(FractalEngine_Math PRIVATE
    MathUtils.cpp
    Noise.cpp
    Random.cpp
    Transform.cpp
)

target_include_directories(FractalEngine_Math PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/math
)

if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Math PRIVATE -Wall -Wextra)
endif()
