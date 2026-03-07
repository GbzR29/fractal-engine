# =====================
# Platform Module
# =====================
add_library(FractalEngine_Platform STATIC)

target_sources(FractalEngine_Platform PRIVATE
    Platform.cpp
)

target_include_directories(FractalEngine_Platform PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/platform
)

if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Platform PRIVATE -Wall -Wextra)
endif()
