# =====================
# Ui Module
# =====================
add_library(FractalEngine_Ui STATIC)

target_sources(FractalEngine_Ui PRIVATE
    Button.cpp
    Panel.cpp
    Text.cpp
    UIManager.cpp
    Widget.cpp
)

target_include_directories(FractalEngine_Ui PUBLIC
    ${CMAKE_SOURCE_DIR}/include/fractal_engine/ui
)
target_link_libraries(FractalEngine_Ui PUBLIC
    FractalEngine_Graphics
)


if(MINGW OR UNIX)
    target_compile_options(FractalEngine_Ui PRIVATE -Wall -Wextra)
endif()
