/**
 * @file AssetLoader.hpp
 * @brief High-level asset loading façade — resolves paths and delegates to @ref AssetManager.
 *
 * Usage:
 * @code
 *   AssetLoader::setAssetsRoot("assets/");
 *   auto model = AssetLoader::loadModel("models/character.fbx");
 * @endcode
 *
 * All paths passed to this class may be relative (resolved against the assets root) or
 * absolute (used as-is).
 */
#pragma once

#include "Texture.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

class Model;
class Shader;

/// Drains the @ref AssetManager async queue.  Safe to call every frame after the GL context exists.
void flushPendingAssetLoads();

/// @brief Path-resolving façade over @ref AssetManager.
class AssetLoader {
public:
    /**
     * @brief Sets the root directory for resolving relative asset paths.
     * @param root  Absolute or working-directory-relative path to the assets folder.
     */
    static void setAssetsRoot(std::filesystem::path root);

    /// @return The currently configured assets root path.
    static const std::filesystem::path& assetsRoot();

    /**
     * @brief Resolves an asset path.
     * @param relativeOrAbsolute  A relative path is joined with @ref assetsRoot(); absolute paths
     *                            are returned unchanged.
     * @return The canonical absolute path.
     */
    static std::filesystem::path resolve(std::string_view relativeOrAbsolute);

    /**
     * @brief Loads a 3-D model, using the cache if already loaded.
     * @param relativePath  Asset-relative path (e.g. "models/character.fbx").
     * @return Loaded model, or @c nullptr on failure.
     */
    static std::shared_ptr<Model> loadModel(const std::string& relativePath);

    /**
     * @brief Loads a texture, using the cache if already loaded.
     * @param relativePath  Asset-relative path (e.g. "textures/albedo.png").
     * @param type          Semantic type used for PBR slot binding.
     * @param flipY         Flip vertically for OpenGL coordinate convention.
     * @param sRGB          Treat as sRGB (gamma-correct albedo / emissive).
     * @return Loaded texture, or @c nullptr on failure.
     */
    static std::shared_ptr<Texture> loadTexture(const std::string& relativePath,
                                                 TextureType type  = TextureType::Unknown,
                                                 bool        flipY = true,
                                                 bool        sRGB  = false);

    /**
     * @brief Compiles and links a shader from asset-relative source paths.
     * @param shader        Output shader object.
     * @param vertRelative  Relative path to the vertex shader source.
     * @param fragRelative  Relative path to the fragment shader source.
     * @param geomRelative  Optional relative path to a geometry shader source.
     * @return @c true on success.
     */
    static bool loadShader(Shader&            shader,
                           const std::string& vertRelative,
                           const std::string& fragRelative,
                           const std::string& geomRelative = "");

private:
    static std::filesystem::path s_root;
};
