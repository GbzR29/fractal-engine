# =====================
# Win32 Module
# =====================
add_library(FractalEngine_Platform_Win32 STATIC)

target_sources(FractalEngine_Platform_Win32 PRIVATE
    Win32Input.cpp
    Win32Window.cpp
)

target_include_directories(FractalEngine_Platform_Win32 PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/platform/win32
)

if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Platform_Win32 PRIVATE -Wall -Wextra)
endif()
