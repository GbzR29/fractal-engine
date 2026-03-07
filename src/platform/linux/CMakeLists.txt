# =====================
# Linux Module
# =====================
add_library(FractalEngine_Platform_Linux STATIC)

target_sources(FractalEngine_Platform_Linux PRIVATE
    LinuxInput.cpp
    LinuxWindow.cpp
)

target_include_directories(FractalEngine_Platform_Linux PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/platform/linux
)

if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Platform_Linux PRIVATE -Wall -Wextra)
endif()
