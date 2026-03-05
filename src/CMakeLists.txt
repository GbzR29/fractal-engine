# CMakeLists.txt for Sources
# This file aggregates all module subdirectories

# Add module subdirectories
add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(input)
add_subdirectory(world)
add_subdirectory(scene)
add_subdirectory(renderer)
add_subdirectory(ui)

# Gather all source files
file(GLOB_RECURSE SOURCES
    "*.cpp"
    "*.c"
)

# Create dependencies on all modules through object libraries
add_library(fractal_engine_all_modules OBJECT)
target_link_libraries(fractal_engine_all_modules
    PRIVATE
    fractal_engine_core
    fractal_engine_graphics
    fractal_engine_input
    fractal_engine_world
    fractal_engine_scene
    fractal_engine_renderer
    fractal_engine_ui
)
