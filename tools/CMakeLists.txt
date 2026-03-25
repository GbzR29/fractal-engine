# tools/CMakeLists.txt

# ─────────────────────────────────────────────────────────────────────────────
#  Asset Pipeline
# ─────────────────────────────────────────────────────────────────────────────
add_executable(AssetImporter
    asset_pipeline/AssetImporter.cpp
)
target_link_libraries(AssetImporter PRIVATE FractalEngineLib)

# ─────────────────────────────────────────────────────────────────────────────
#  Shader Compiler
# ─────────────────────────────────────────────────────────────────────────────
add_executable(ShaderCompiler
    shader_compiler/ShaderCompiler.cpp
)
target_link_libraries(ShaderCompiler PRIVATE FractalEngineLib)

# ─────────────────────────────────────────────────────────────────────────────
#  Editor
# ─────────────────────────────────────────────────────────────────────────────
if (FE_BUILD_EDITOR)
    add_executable(FractalEditor
        editor/Editor.cpp
    )
    target_link_libraries(FractalEditor PRIVATE FractalEngineLib imgui)
endif()