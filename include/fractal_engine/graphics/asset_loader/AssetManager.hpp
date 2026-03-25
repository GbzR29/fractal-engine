#pragma once
#include "Model.hpp"
#include "Texture.hpp"
#include "ModelLoader.hpp"

#include <filesystem>
#include <future>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <functional>
#include <queue>
#include <atomic>
#include <variant>
#include <chrono>
#include <iostream>

namespace fs = std::filesystem;

// ──────────────────────────────────────────────────────────────────────────
//  LoadResult — sent from async threads to the GL main thread
// ──────────────────────────────────────────────────────────────────────────
struct LoadResult {
    std::string                  key;
    std::shared_ptr<Model>       model;   // non-null if a Model was loaded
    std::shared_ptr<Texture>     texture; // non-null if a Texture was loaded
    std::function<void()>        onReady; // optional callback
};

// ──────────────────────────────────────────────────────────────────────────
//  AssetManager
// ──────────────────────────────────────────────────────────────────────────
class AssetManager {
public:
    // ── Singleton ────────────────────────────────────────────────
    static AssetManager& get() {
        static AssetManager instance;
        return instance;
    }

    AssetManager(const AssetManager&)            = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // ──────────────────────────────────────────────────────────────
    //  Synchronous load  (blocks caller, returns immediately)
    // ──────────────────────────────────────────────────────────────
    std::shared_ptr<Model> loadModel(const std::string& path) {
        {
            std::lock_guard lock(m_modelMtx);
            auto it = m_models.find(path);
            if (it != m_models.end()) return it->second;
        }

        auto model = ModelLoader::load(path);
        if (model) {
            model->uploadToGPU(); // sync path — we're on the GL thread
            std::lock_guard lock(m_modelMtx);
            m_models[path] = model;
        }
        return model;
    }

    std::shared_ptr<Texture> loadTexture(const std::string& path,
                                          TextureType type = TextureType::Unknown,
                                          bool flipY = true,
                                          bool sRGB  = false)
    {
        {
            std::lock_guard lock(m_texMtx);
            auto it = m_textures.find(path);
            if (it != m_textures.end()) return it->second;
        }

        auto tex = Texture::load(path, type, flipY, sRGB);
        if (tex) {
            std::lock_guard lock(m_texMtx);
            m_textures[path] = tex;
        }
        return tex;
    }

    // ──────────────────────────────────────────────────────────────
    //  Asynchronous load
    //  onReady is called on the GL thread (inside flushPendingLoads)
    // ──────────────────────────────────────────────────────────────
    void loadModelAsync(const std::string& path,
                        std::function<void(std::shared_ptr<Model>)> onReady = nullptr)
    {
        {
            std::lock_guard lock(m_modelMtx);
            if (m_models.count(path)) {
                if (onReady) onReady(m_models[path]);
                return;
            }
        }

        std::thread([this, path, cb = std::move(onReady)]() {
            auto model = ModelLoader::load(path); // CPU work — no GL calls

            LoadResult r;
            r.key   = path;
            r.model = model;
            if (cb) r.onReady = [cb, model]() { cb(model); };

            std::lock_guard lock(m_resultMtx);
            m_pendingResults.push(std::move(r));
        }).detach();
    }

    void loadTextureAsync(const std::string& path,
                          std::function<void(std::shared_ptr<Texture>)> onReady = nullptr,
                          TextureType type = TextureType::Unknown,
                          bool flipY = true, bool sRGB = false)
    {
        {
            std::lock_guard lock(m_texMtx);
            if (m_textures.count(path)) {
                if (onReady) onReady(m_textures[path]);
                return;
            }
        }

        std::thread([this, path, type, flipY, sRGB, cb = std::move(onReady)]() {
            auto tex = Texture::load(path, type, flipY, sRGB);

            LoadResult r;
            r.key     = path;
            r.texture = tex;
            if (cb) r.onReady = [cb, tex]() { cb(tex); };

            std::lock_guard lock(m_resultMtx);
            m_pendingResults.push(std::move(r));
        }).detach();
    }

    // ──────────────────────────────────────────────────────────────
    //  Must be called once per frame on the GL thread.
    //  Uploads GPU data for all finished background loads.
    // ──────────────────────────────────────────────────────────────
    void flushPendingLoads() {
        std::queue<LoadResult> local;
        {
            std::lock_guard lock(m_resultMtx);
            std::swap(local, m_pendingResults);
        }

        while (!local.empty()) {
            auto& r = local.front();

            if (r.model) {
                r.model->uploadToGPU();
                std::lock_guard lock(m_modelMtx);
                m_models[r.key] = r.model;
            }

            if (r.texture) {
                // Texture upload already happened inside Texture::load
                // (stb + glTexImage2D are bundled), but if you split them
                // in the future, do GPU upload here.
                std::lock_guard lock(m_texMtx);
                m_textures[r.key] = r.texture;
            }

            if (r.onReady) r.onReady();

            local.pop();
        }
    }

    // ──────────────────────────────────────────────────────────────
    //  Hot-reload  (call from a background thread or each frame)
    //
    //  Checks file modification times; reloads changed assets.
    //  Safe: queues results through the same pending-results channel.
    // ──────────────────────────────────────────────────────────────
    void startHotReloadWatcher(std::chrono::milliseconds pollInterval =
                                    std::chrono::milliseconds(500))
    {
        if (m_watcherRunning.exchange(true)) return; // already running

        std::thread([this, pollInterval]() {
            using Clock = std::filesystem::file_time_type::clock;

            // Build initial timestamps
            std::unordered_map<std::string, fs::file_time_type> stamps;
            auto snapshot = [&]() {
                std::lock_guard lm(m_modelMtx);
                std::lock_guard lt(m_texMtx);
                for (auto& [k, _] : m_models)   stamps[k] = lastWrite(k);
                for (auto& [k, _] : m_textures)  stamps[k] = lastWrite(k);
            };
            snapshot();

            while (m_watcherRunning) {
                std::this_thread::sleep_for(pollInterval);

                // Check models
                {
                    std::lock_guard lock(m_modelMtx);
                    for (auto& [path, _] : m_models) {
                        auto t = lastWrite(path);
                        if (t != stamps[path]) {
                            stamps[path] = t;
                            std::cout << "[HotReload] Reloading model: " << path << "\n";
                            auto model = ModelLoader::load(path);
                            if (model) {
                                LoadResult r; r.key = path; r.model = model;
                                std::lock_guard rl(m_resultMtx);
                                m_pendingResults.push(std::move(r));
                            }
                        }
                    }
                }

                // Check textures
                {
                    std::lock_guard lock(m_texMtx);
                    for (auto& [path, tex] : m_textures) {
                        auto t = lastWrite(path);
                        if (t != stamps[path]) {
                            stamps[path] = t;
                            std::cout << "[HotReload] Reloading texture: " << path << "\n";
                            auto type  = tex ? tex->type  : TextureType::Unknown;
                            auto newTex = Texture::load(path, type);
                            if (newTex) {
                                LoadResult r; r.key = path; r.texture = newTex;
                                std::lock_guard rl(m_resultMtx);
                                m_pendingResults.push(std::move(r));
                            }
                        }
                    }
                }
            }
        }).detach();
    }

    void stopHotReloadWatcher() { m_watcherRunning = false; }

    // ──────────────────────────────────────────────────────────────
    //  Cache queries
    // ──────────────────────────────────────────────────────────────
    std::shared_ptr<Model>   getModel  (const std::string& key) const {
        std::lock_guard l(m_modelMtx);
        auto it = m_models.find(key); return it != m_models.end() ? it->second : nullptr;
    }
    std::shared_ptr<Texture> getTexture(const std::string& key) const {
        std::lock_guard l(m_texMtx);
        auto it = m_textures.find(key); return it != m_textures.end() ? it->second : nullptr;
    }

    void evictModel  (const std::string& key) { std::lock_guard l(m_modelMtx);   m_models.erase(key);   }
    void evictTexture(const std::string& key) { std::lock_guard l(m_texMtx);     m_textures.erase(key); }
    void clearAll()                           { std::lock_guard lm(m_modelMtx), lt(m_texMtx);
                                                m_models.clear(); m_textures.clear(); }

    size_t modelCount()   const { std::lock_guard l(m_modelMtx);  return m_models.size();   }
    size_t textureCount() const { std::lock_guard l(m_texMtx);    return m_textures.size(); }

private:
    AssetManager()  = default;
    ~AssetManager() { stopHotReloadWatcher(); }

    static fs::file_time_type lastWrite(const std::string& path) {
        std::error_code ec;
        auto t = fs::last_write_time(path, ec);
        return ec ? fs::file_time_type{} : t;
    }

    // Caches
    mutable std::mutex                                       m_modelMtx;
    std::unordered_map<std::string, std::shared_ptr<Model>>  m_models;

    mutable std::mutex                                         m_texMtx;
    std::unordered_map<std::string, std::shared_ptr<Texture>>  m_textures;

    // Async results queue (write: bg threads; read: GL thread)
    std::mutex             m_resultMtx;
    std::queue<LoadResult> m_pendingResults;

    // Hot-reload watcher
    std::atomic<bool> m_watcherRunning{ false };
};