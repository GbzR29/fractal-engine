/**
 * @file AssetManager.hpp
 * @brief Thread-safe asset cache and async loader for models and textures.
 *
 * Assets are keyed by their absolute path string.  Duplicate load requests return
 * the cached @c shared_ptr without re-reading the file.
 *
 * Async workflow:
 * 1. Call @ref loadModelAsync() / @ref loadTextureAsync() from any thread.
 * 2. Call @ref flushPendingLoads() on the GL thread once per frame to complete GPU uploads.
 *
 * Prefer the @ref AssetLoader façade for path resolution and high-level access.
 */
#pragma once

#include "Model.hpp"
#include "Texture.hpp"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

/// @brief Result of an async load operation — queued until @ref AssetManager::flushPendingLoads().
struct LoadResult {
    std::string              key;      ///< Cache key (absolute path).
    std::shared_ptr<Model>   model;    ///< Loaded model (null if this is a texture load).
    std::shared_ptr<Texture> texture;  ///< Loaded texture (null if this is a model load).
    std::function<void()>    onReady;  ///< Optional callback invoked after GPU upload.
};

/// @brief Singleton asset cache with optional async loading and hot-reload watching.
class AssetManager {
public:
    /// @return The global AssetManager singleton.
    static AssetManager& get();

    AssetManager(const AssetManager&)            = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    /**
     * @brief Loads a model synchronously (blocks the calling thread).
     * @param path  Absolute path to the model file.
     * @return Cached or newly loaded model, or @c nullptr on failure.
     */
    std::shared_ptr<Model> loadModel(const std::string& path);

    /**
     * @brief Loads a texture synchronously (blocks the calling thread).
     * @param path   Absolute path to the image file.
     * @param type   Semantic texture type for PBR binding.
     * @param flipY  Flip image vertically (OpenGL expects origin at bottom-left).
     * @param sRGB   Upload as sRGB (gamma-correct for albedo / emissive maps).
     * @return Cached or newly loaded texture, or @c nullptr on failure.
     */
    std::shared_ptr<Texture> loadTexture(const std::string& path,
                                         TextureType type  = TextureType::Unknown,
                                         bool        flipY = true,
                                         bool        sRGB  = false);

    /**
     * @brief Launches an async model load on a worker thread.
     * @param path     Absolute path to the model file.
     * @param onReady  Optional callback invoked on the GL thread after GPU upload.
     */
    void loadModelAsync(const std::string& path,
                        std::function<void(std::shared_ptr<Model>)> onReady = nullptr);

    /**
     * @brief Launches an async texture load on a worker thread.
     * @param path     Absolute path to the image file.
     * @param onReady  Optional callback invoked on the GL thread after GPU upload.
     * @param type     Semantic texture type.
     * @param flipY    Flip image vertically.
     * @param sRGB     Upload as sRGB.
     */
    void loadTextureAsync(const std::string& path,
                          std::function<void(std::shared_ptr<Texture>)> onReady = nullptr,
                          TextureType type  = TextureType::Unknown,
                          bool        flipY = true,
                          bool        sRGB  = false);

    /// Uploads pending async results to the GPU.  Must be called on the GL thread each frame.
    void flushPendingLoads();

    /**
     * @brief Starts a background thread that polls asset files for modifications.
     * @param pollInterval  How often to check file timestamps.
     */
    void startHotReloadWatcher(std::chrono::milliseconds pollInterval =
                                   std::chrono::milliseconds(500));

    /// Stops the hot-reload watcher thread.
    void stopHotReloadWatcher();

    /// @return Cached model by key, or @c nullptr if not in cache.
    std::shared_ptr<Model>   getModel  (const std::string& key) const;

    /// @return Cached texture by key, or @c nullptr if not in cache.
    std::shared_ptr<Texture> getTexture(const std::string& key) const;

    void evictModel  (const std::string& key); ///< Removes a model from the cache.
    void evictTexture(const std::string& key); ///< Removes a texture from the cache.
    void clearAll();                            ///< Clears all cached models and textures.

    size_t modelCount()   const; ///< @return Number of models currently in cache.
    size_t textureCount() const; ///< @return Number of textures currently in cache.

private:
    AssetManager();
    ~AssetManager();

    static fs::file_time_type lastWrite(const std::string& path);

    mutable std::mutex                                       m_modelMtx;
    std::unordered_map<std::string, std::shared_ptr<Model>> m_models;

    mutable std::mutex                                         m_texMtx;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;

    std::mutex             m_resultMtx;
    std::queue<LoadResult> m_pendingResults;

    std::atomic<bool> m_watcherRunning{ false };
};
