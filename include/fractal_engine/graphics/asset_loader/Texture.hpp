/**
 * @file Texture.hpp
 * @brief OpenGL 2-D texture with optional two-phase CPU/GPU loading for async use.
 *
 * For background (worker-thread) loading:
 * 1. Call @ref decodeFromFile() on the worker thread — decodes image pixels into @c m_staging.
 * 2. Call @ref uploadToGPU() on the GL thread — creates the OpenGL texture from staging data.
 *
 * For synchronous loading (GL thread):
 * - Use @ref load() for a single combined decode+upload call.
 *
 * 1×1 fallback textures (@ref white(), @ref black(), @ref defaultNormal()) are cached as singletons
 * and safe to use as default values for missing maps.
 */
#pragma once

#include <glad/glad.h>
#include <memory>
#include <string>
#include <vector>

/// @brief Semantic type of a texture, used by @ref Material for PBR slot binding.
enum class TextureType {
    Albedo,    ///< Base colour (sRGB).
    Normal,    ///< Tangent-space normal map (linear).
    Metallic,  ///< Metallic channel (linear, R channel used).
    Roughness, ///< Roughness channel (linear, R channel used).
    AO,        ///< Ambient occlusion (linear, R channel used).
    Emissive,  ///< Emissive colour (sRGB or HDR).
    Unknown    ///< Type not specified.
};

class Texture {
public:
    GLuint      id       = 0;
    int         width    = 0;
    int         height   = 0;
    int         channels = 0;
    std::string path;
    TextureType type     = TextureType::Unknown;

    Texture() = default;
    ~Texture() { if (id) glDeleteTextures(1, &id); }

    // Non-copyable, movable
    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& o) noexcept
        : id(o.id), width(o.width), height(o.height), channels(o.channels),
          path(std::move(o.path)), type(o.type), m_staging(std::move(o.m_staging)),
          m_stagingW(o.m_stagingW), m_stagingH(o.m_stagingH), m_stagingCh(o.m_stagingCh),
          m_stagingSRGB(o.m_stagingSRGB)
    {
        o.id = 0;
        o.m_stagingW = o.m_stagingH = o.m_stagingCh = 0;
        o.m_stagingSRGB = false;
    }

    bool isValid() const { return id != 0; }
    bool needsGpuUpload() const { return id == 0 && !m_staging.empty(); }

    void bind(GLuint slot = 0) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    /// Decode image on the CPU only (safe from worker threads). Call `uploadToGPU` on the GL thread.
    static std::shared_ptr<Texture> decodeFromFile(const std::string& path,
                                                   TextureType type = TextureType::Unknown,
                                                   bool flipY = true,
                                                   bool sRGB  = false);

    /// Decode + upload immediately (must run on the GL context thread).
    static std::shared_ptr<Texture> load(const std::string& path,
                                         TextureType type = TextureType::Unknown,
                                         bool flipY = true,
                                         bool sRGB  = false);

    void uploadToGPU();

    // 1x1 fallback textures (require active GL context when first called)
    static std::shared_ptr<Texture> white();
    static std::shared_ptr<Texture> black();
    static std::shared_ptr<Texture> defaultNormal(); // (0.5, 0.5, 1.0)

private:
    static std::shared_ptr<Texture> solidCached(const char* tag,
                                                uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    static std::shared_ptr<Texture> createSolid(const char* tag,
                                                uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    std::vector<unsigned char> m_staging;
    int                        m_stagingW    = 0;
    int                        m_stagingH    = 0;
    int                        m_stagingCh   = 0;
    bool                       m_stagingSRGB = false;

    void uploadRaw(const unsigned char* data, int w, int h, int ch, bool sRGB);
};