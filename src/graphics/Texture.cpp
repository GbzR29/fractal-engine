#include "Texture.hpp"

#include <algorithm>
#include <iostream>
#include <mutex>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace {

GLenum channelsToFormat(int ch)
{
    switch (ch) {
        case 1: return GL_RED;
        case 2: return GL_RG;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: return GL_RGBA;
    }
}

GLenum channelsToInternalFormat(int ch, bool sRGB)
{
    if (sRGB)
        return (ch == 4) ? GL_SRGB8_ALPHA8 : GL_SRGB8;
    switch (ch) {
        case 1: return GL_R8;
        case 2: return GL_RG8;
        case 3: return GL_RGB8;
        case 4: return GL_RGBA8;
        default: return GL_RGBA8;
    }
}

} // namespace

void Texture::uploadRaw(const unsigned char* data, int w, int h, int ch, bool sRGB)
{
    if (id)
        glDeleteTextures(1, &id);

    GLenum fmt    = channelsToFormat(ch);
    GLenum intFmt = channelsToInternalFormat(ch, sRGB);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, intFmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
    if (maxAniso > 0.0f)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY,
                        std::min(maxAniso, 8.0f));

    glBindTexture(GL_TEXTURE_2D, 0);

    width    = w;
    height   = h;
    channels = ch;
}

void Texture::uploadToGPU()
{
    if (id != 0 || m_staging.empty())
        return;

    uploadRaw(m_staging.data(), m_stagingW, m_stagingH, m_stagingCh, m_stagingSRGB);
    m_staging.clear();
    m_staging.shrink_to_fit();
    m_stagingW = m_stagingH = m_stagingCh = 0;
    m_stagingSRGB                         = false;
}

std::shared_ptr<Texture> Texture::decodeFromFile(const std::string& path,
                                                 TextureType   type,
                                                 bool          flipY,
                                                 bool          sRGB)
{
    stbi_set_flip_vertically_on_load_thread(flipY ? 1 : 0);

    int w = 0, h = 0, ch = 0;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (!data) {
        std::cerr << "[Texture] Falha ao decodificar: " << path
                  << " — " << stbi_failure_reason() << "\n";
        return nullptr;
    }

    const size_t bytes = static_cast<size_t>(w) * static_cast<size_t>(h) * static_cast<size_t>(ch);
    auto tex            = std::make_shared<Texture>();
    tex->path           = path;
    tex->type           = type;
    tex->m_stagingW     = w;
    tex->m_stagingH     = h;
    tex->m_stagingCh    = ch;
    tex->m_stagingSRGB  = sRGB;
    tex->m_staging.assign(data, data + bytes);
    stbi_image_free(data);

    return tex;
}

std::shared_ptr<Texture> Texture::load(const std::string& path,
                                         TextureType   type,
                                         bool          flipY,
                                         bool          sRGB)
{
    auto tex = decodeFromFile(path, type, flipY, sRGB);
    if (tex)
        tex->uploadToGPU();
    return tex;
}

std::shared_ptr<Texture> Texture::createSolid(const char* tag,
                                              uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    auto t  = std::make_shared<Texture>();
    t->path = tag;
    t->type = TextureType::Unknown;
    uint8_t px[4] = { r, g, b, a };
    t->uploadRaw(px, 1, 1, 4, false);
    return t;
}

std::shared_ptr<Texture> Texture::solidCached(const char* tag,
                                              uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    static std::mutex                                              mtx;
    static std::unordered_map<std::string, std::weak_ptr<Texture>> cache;

    std::lock_guard lock(mtx);
    if (auto p = cache[tag].lock())
        return p;
    auto tex = createSolid(tag, r, g, b, a);
    cache[tag] = tex;
    return tex;
}

std::shared_ptr<Texture> Texture::white()
{
    return solidCached("__fe_tex_white", 255, 255, 255, 255);
}

std::shared_ptr<Texture> Texture::black()
{
    return solidCached("__fe_tex_black", 0, 0, 0, 255);
}

std::shared_ptr<Texture> Texture::defaultNormal()
{
    return solidCached("__fe_tex_default_normal", 128, 128, 255, 255);
}
