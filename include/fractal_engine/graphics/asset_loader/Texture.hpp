#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <memory>

enum class TextureType {
    Albedo,
    Normal,
    Metallic,
    Roughness,
    AO,
    Emissive,
    Unknown
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
    Texture(Texture&& o) noexcept : id(o.id), width(o.width), height(o.height),
        channels(o.channels), path(std::move(o.path)), type(o.type) { o.id = 0; }

    bool isValid() const { return id != 0; }

    void bind(GLuint slot = 0) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    static std::shared_ptr<Texture> load(const std::string& path,
                                         TextureType type = TextureType::Unknown,
                                         bool flipY = true,
                                         bool sRGB  = false);

    // 1x1 fallback textures
    static std::shared_ptr<Texture> white();
    static std::shared_ptr<Texture> black();
    static std::shared_ptr<Texture> defaultNormal(); // (0.5, 0.5, 1.0)

private:
    static GLuint upload(unsigned char* data, int w, int h, int ch, bool sRGB);
};