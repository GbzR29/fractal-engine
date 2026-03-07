# =====================
# GLAD - OpenGL Loader
# =====================

add_library(glad STATIC
    glad.c
)

target_include_directories(glad PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/third_party
)

# Allow undefined references to be resolved later
target_link_options(glad PRIVATE -fcommon)

set_target_properties(glad PROPERTIES
    LINKER_LANGUAGE C
    POSITION_INDEPENDENT_CODE ON
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    COMPILE_FLAGS "-fPIC -Wno-all"
)

