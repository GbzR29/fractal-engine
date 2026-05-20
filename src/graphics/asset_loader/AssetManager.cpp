#include "AssetManager.hpp"

#include "ModelLoader.hpp"

#include <iostream>
#include <thread>

AssetManager& AssetManager::get()
{
    static AssetManager instance;
    return instance;
}

AssetManager::AssetManager()  = default;
AssetManager::~AssetManager() { stopHotReloadWatcher(); }

std::shared_ptr<Model> AssetManager::loadModel(const std::string& path)
{
    {
        std::lock_guard lock(m_modelMtx);
        auto            it = m_models.find(path);
        if (it != m_models.end()) return it->second;
    }

    auto model = ModelLoader::load(path);
    if (model) {
        model->uploadToGPU();
        std::lock_guard lock(m_modelMtx);
        m_models[path] = model;
    }
    return model;
}

std::shared_ptr<Texture> AssetManager::loadTexture(const std::string& path,
                                                   TextureType   type,
                                                   bool          flipY,
                                                   bool          sRGB)
{
    {
        std::lock_guard lock(m_texMtx);
        auto            it = m_textures.find(path);
        if (it != m_textures.end()) return it->second;
    }

    auto tex = Texture::load(path, type, flipY, sRGB);
    if (tex) {
        std::lock_guard lock(m_texMtx);
        m_textures[path] = tex;
    }
    return tex;
}

void AssetManager::loadModelAsync(const std::string& path,
                                  std::function<void(std::shared_ptr<Model>)> onReady)
{
    {
        std::lock_guard lock(m_modelMtx);
        if (m_models.count(path)) {
            if (onReady) onReady(m_models[path]);
            return;
        }
    }

    std::thread([this, path, cb = std::move(onReady)]() {
        auto model = ModelLoader::load(path);

        LoadResult r;
        r.key   = path;
        r.model = model;
        if (cb) r.onReady = [cb, model]() { cb(model); };

        std::lock_guard lock(m_resultMtx);
        m_pendingResults.push(std::move(r));
    }).detach();
}

void AssetManager::loadTextureAsync(const std::string& path,
                                    std::function<void(std::shared_ptr<Texture>)> onReady,
                                    TextureType type,
                                    bool        flipY,
                                    bool        sRGB)
{
    {
        std::lock_guard lock(m_texMtx);
        if (m_textures.count(path)) {
            if (onReady) onReady(m_textures[path]);
            return;
        }
    }

    std::thread([this, path, type, flipY, sRGB, cb = std::move(onReady)]() {
        auto tex = Texture::decodeFromFile(path, type, flipY, sRGB);

        LoadResult r;
        r.key     = path;
        r.texture = tex;
        if (cb) r.onReady = [cb, tex]() { cb(tex); };

        std::lock_guard lock(m_resultMtx);
        m_pendingResults.push(std::move(r));
    }).detach();
}

void AssetManager::flushPendingLoads()
{
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
            r.texture->uploadToGPU();
            std::lock_guard lock(m_texMtx);
            m_textures[r.key] = r.texture;
        }

        if (r.onReady) r.onReady();

        local.pop();
    }
}

void AssetManager::startHotReloadWatcher(std::chrono::milliseconds pollInterval)
{
    if (m_watcherRunning.exchange(true)) return;

    std::thread([this, pollInterval]() {
        std::unordered_map<std::string, fs::file_time_type> stamps;
        auto snapshot = [&]() {
            std::lock_guard lm(m_modelMtx);
            std::lock_guard lt(m_texMtx);
            for (auto& [k, _] : m_models) stamps[k] = lastWrite(k);
            for (auto& [k, _] : m_textures) stamps[k] = lastWrite(k);
        };
        snapshot();

        while (m_watcherRunning) {
            std::this_thread::sleep_for(pollInterval);

            {
                std::lock_guard lock(m_modelMtx);
                for (auto& [path, _] : m_models) {
                    auto t = lastWrite(path);
                    if (t != stamps[path]) {
                        stamps[path] = t;
                        std::cout << "[HotReload] Reloading model: " << path << "\n";
                        auto model = ModelLoader::load(path);
                        if (model) {
                            LoadResult r;
                            r.key   = path;
                            r.model = model;
                            std::lock_guard rl(m_resultMtx);
                            m_pendingResults.push(std::move(r));
                        }
                    }
                }
            }

            {
                std::lock_guard lock(m_texMtx);
                for (auto& [path, tex] : m_textures) {
                    auto t = lastWrite(path);
                    if (t != stamps[path]) {
                        stamps[path] = t;
                        std::cout << "[HotReload] Reloading texture: " << path << "\n";
                        auto type   = tex ? tex->type : TextureType::Unknown;
                        auto newTex = Texture::decodeFromFile(path, type, true, false);
                        if (newTex) {
                            LoadResult r;
                            r.key     = path;
                            r.texture = newTex;
                            std::lock_guard rl(m_resultMtx);
                            m_pendingResults.push(std::move(r));
                        }
                    }
                }
            }
        }
    }).detach();
}

void AssetManager::stopHotReloadWatcher() { m_watcherRunning = false; }

std::shared_ptr<Model> AssetManager::getModel(const std::string& key) const
{
    std::lock_guard l(m_modelMtx);
    auto            it = m_models.find(key);
    return it != m_models.end() ? it->second : nullptr;
}

std::shared_ptr<Texture> AssetManager::getTexture(const std::string& key) const
{
    std::lock_guard l(m_texMtx);
    auto            it = m_textures.find(key);
    return it != m_textures.end() ? it->second : nullptr;
}

void AssetManager::evictModel(const std::string& key)
{
    std::lock_guard l(m_modelMtx);
    m_models.erase(key);
}

void AssetManager::evictTexture(const std::string& key)
{
    std::lock_guard l(m_texMtx);
    m_textures.erase(key);
}

void AssetManager::clearAll()
{
    std::lock_guard lm(m_modelMtx), lt(m_texMtx);
    m_models.clear();
    m_textures.clear();
}

size_t AssetManager::modelCount() const
{
    std::lock_guard l(m_modelMtx);
    return m_models.size();
}

size_t AssetManager::textureCount() const
{
    std::lock_guard l(m_texMtx);
    return m_textures.size();
}

fs::file_time_type AssetManager::lastWrite(const std::string& path)
{
    std::error_code ec;
    auto            t = fs::last_write_time(path, ec);
    return ec ? fs::file_time_type{} : t;
}
