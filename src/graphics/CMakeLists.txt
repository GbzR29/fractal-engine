# src/graphics/CMakeLists.txt

set(FE_GRAPHICS_ASSET_LOADER
    asset_loader/Mesh.cpp
    asset_loader/Model.cpp
    asset_loader/Animation.cpp
    asset_loader/AssetLoader.cpp
    asset_loader/AssetManager.cpp
    asset_loader/PrimitiveMesh.cpp
)

if (FE_USE_ASSIMP)
    list(APPEND FE_GRAPHICS_ASSET_LOADER asset_loader/ModelLoader.cpp)
else()
    list(APPEND FE_GRAPHICS_ASSET_LOADER asset_loader/ModelLoaderDisabled.cpp)
endif()

target_sources(FractalEngineLib PRIVATE
    EditorCamera.cpp
    Material.cpp
    Shader.cpp
    SceneRenderer.cpp
    SkyRenderer.cpp
    Texture.cpp
    TextureLoader.cpp
    ViewportPanel.cpp
    GLUtils.cpp
    ${FE_GRAPHICS_ASSET_LOADER}
)