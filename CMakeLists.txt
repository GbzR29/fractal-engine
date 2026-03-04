cmake_minimum_required(VERSION 4.2.1)

project(FractalEngine LANGUAGES CXX C)

# =====================
# C++ Standard
# =====================

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# =====================
# Sources (todos os .cpp dentro de src)
# =====================

file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

# =====================
# Includes
# =====================

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# =====================
# Library directory
# =====================

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
)

# =====================
# Link libraries
# =====================

target_link_libraries(${PROJECT_NAME}
    glfw3dll
    glfw3
    lua55
    SDL3
    opengl32
    gdi32
    user32
    kernel32
)

# =====================
# Flags MinGW
# =====================

if(MINGW)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -g
    )
endif()

# =====================
# Copiar DLL runtime
# =====================

file(GLOB RUNTIME_DLLS
    ${CMAKE_SOURCE_DIR}/bin/*.dll
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${RUNTIME_DLLS}
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)