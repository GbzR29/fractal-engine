# =====================
# Macos Module
# =====================
add_library(FractalEngine_Platform_Macos STATIC)

target_sources(FractalEngine_Platform_Macos PRIVATE
    MacOSInput.cpp
    MacOSWindow.cpp
)

target_include_directories(FractalEngine_Platform_Macos PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/platform/macos
)

if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Platform_Macos PRIVATE -Wall -Wextra)
endif()
