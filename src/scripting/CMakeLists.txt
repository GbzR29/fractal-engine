# =====================
# Scripting Module
# =====================
add_library(FractalEngine_Scripting STATIC)

target_sources(FractalEngine_Scripting PRIVATE
    LuaBinding.cpp
    ScriptContext.cpp
    ScriptEngine.cpp
)

target_include_directories(FractalEngine_Scripting PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/scripting
)

if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Scripting PRIVATE -Wall -Wextra)
endif()
