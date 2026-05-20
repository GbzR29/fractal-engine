#include "AssetLoader.hpp"

#include "AssetManager.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "AssetsRoot.hpp"

void flushPendingAssetLoads() { AssetManager::get().flushPendingLoads(); }

std::filesystem::path AssetLoader::s_root = FE_ASSETS_DEFAULT_ROOT;

void AssetLoader::setAssetsRoot(std::filesystem::path root) { s_root = std::move(root); }

const std::filesystem::path& AssetLoader::assetsRoot() { return s_root; }

std::filesystem::path AssetLoader::resolve(std::string_view relativeOrAbsolute)
{
    std::filesystem::path p(relativeOrAbsolute);
    if (p.is_absolute())
        return p;
    return s_root / p;
}

std::shared_ptr<Model> AssetLoader::loadModel(const std::string& relativePath)
{
    const std::string abs = resolve(relativePath).generic_string();
    return AssetManager::get().loadModel(abs);
}

std::shared_ptr<Texture> AssetLoader::loadTexture(const std::string& relativePath,
                                                    TextureType type,
                                                    bool        flipY,
                                                    bool        sRGB)
{
    const std::string abs = resolve(relativePath).generic_string();
    return AssetManager::get().loadTexture(abs, type, flipY, sRGB);
}

bool AssetLoader::loadShader(Shader&            shader,
                             const std::string& vertRelative,
                             const std::string& fragRelative,
                             const std::string& geomRelative)
{
    const std::string v = resolve(vertRelative).string();
    const std::string f = resolve(fragRelative).string();
    const std::string g = geomRelative.empty() ? std::string{} : resolve(geomRelative).string();
    return shader.LoadFromFiles(v, f, g);
}
