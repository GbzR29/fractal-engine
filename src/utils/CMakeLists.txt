# =====================
# Utils Module
# =====================
add_library(FractalEngine_Utils STATIC)

target_sources(FractalEngine_Utils PRIVATE
    FileSystem.cpp
    Logger.cpp
    MemoryPool.cpp
    StringUtils.cpp
    Timer.cpp
)

target_include_directories(FractalEngine_Utils PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/utils
)

if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Utils PRIVATE -Wall -Wextra)
endif()
