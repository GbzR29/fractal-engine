cmake_minimum_required(VERSION 3.25)

project(FractalEngine LANGUAGES CXX C)

# =====================
# C++ Standard & CMake Settings
# =====================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# =====================
# Build Options
# =====================
option(FE_BUILD_TESTS "Build tests" OFF)
option(FE_BUILD_TOOLS "Build tools" OFF)

# =====================
# Global include directories
# =====================
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/third_party
)

# =====================
# Third-party libraries
# =====================
add_subdirectory(src/third_party)

# =====================
# Main library (all modules)
# =====================
add_subdirectory(src)

# =====================
# Tests
# =====================
if(FE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# =====================
# Tools
# =====================
if(FE_BUILD_TOOLS)
    add_subdirectory(tools)
endif()
