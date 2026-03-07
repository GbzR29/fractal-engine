# =====================
# Audio Module
# =====================
add_library(FractalEngine_Audio STATIC)

target_sources(FractalEngine_Audio PRIVATE
    AudioClip.cpp
    AudioEngine.cpp
    AudioListener.cpp
    AudioSource.cpp
)

target_include_directories(FractalEngine_Audio PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/audio
)

if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Audio PRIVATE -Wall -Wextra)
endif()
