#include "TextureLoader.hpp"
#include <iostream>

// STB_IMAGE_IMPLEMENTATION lives in Texture.cpp (single translation unit)
#include <stb_image.h>

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────
static void ApplySpec(const TextureSpec& spec)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     spec.WrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     spec.WrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, spec.MinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, spec.MagFilter);
}

static GLenum ChannelsToFormat(int channels)
{
    switch (channels)
    {
        case 1: return GL_RED;
        case 2: return GL_RG;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default: return GL_RGBA;
    }
}

static GLenum ChannelsToInternalFormat(int channels, bool sRGB)
{
    if (sRGB)
        return (channels == 4) ? GL_SRGB8_ALPHA8 : GL_SRGB8;

    switch (channels)
    {
        case 1: return GL_R8;
        case 2: return GL_RG8;
        case 3: return GL_RGB8;
        case 4: return GL_RGBA8;
        default: return GL_RGBA8;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Texturas de fallback
// ─────────────────────────────────────────────────────────────────────────────
Texture2D CreateSolidTexture(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    uint8_t data[4] = { r, g, b, a };
    Texture2D tex;
    tex.Width    = 1;
    tex.Height   = 1;
    tex.Channels = 4;

    glGenTextures(1, &tex.ID);
    glBindTexture(GL_TEXTURE_2D, tex.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

Texture2D CreateWhiteTexture()       { return CreateSolidTexture(255, 255, 255, 255); }
Texture2D CreateBlackTexture()       { return CreateSolidTexture(0,   0,   0,   255); }
Texture2D CreateDefaultNormalMap()   { return CreateSolidTexture(128, 128, 255, 255); }

// ─────────────────────────────────────────────────────────────────────────────
//  TextureLoader::Load
// ─────────────────────────────────────────────────────────────────────────────
Texture2D TextureLoader::Load(const std::string& path, const TextureSpec& spec)
{
    stbi_set_flip_vertically_on_load_thread(spec.FlipY ? 1 : 0);

    int w, h, ch;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 0);
    if (!data)
    {
        std::cerr << "[TextureLoader] Falha ao carregar: " << path
                  << " — " << stbi_failure_reason() << "\n";
        return CreateWhiteTexture();   // fallback visível (branco)
    }

    Texture2D tex;
    tex.Width    = w;
    tex.Height   = h;
    tex.Channels = ch;
    tex.Path     = path;

    GLenum fmt      = ChannelsToFormat(ch);
    GLenum intFmt   = ChannelsToInternalFormat(ch, spec.sRGB);

    glGenTextures(1, &tex.ID);
    glBindTexture(GL_TEXTURE_2D, tex.ID);

    glTexImage2D(GL_TEXTURE_2D, 0, intFmt, w, h, 0,
                 fmt, GL_UNSIGNED_BYTE, data);

    if (spec.GenerateMips)
        glGenerateMipmap(GL_TEXTURE_2D);

    ApplySpec(spec);

    // Anisotropia (se disponível)
    float maxAniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
    if (maxAniso > 0.0f)
        glTexParameterf(GL_TEXTURE_2D,
            GL_TEXTURE_MAX_ANISOTROPY, std::min(maxAniso, 8.0f));

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    std::cout << "[TextureLoader] " << path
              << " (" << w << "x" << h << " " << ch << "ch)\n";
    return tex;
}

// ─────────────────────────────────────────────────────────────────────────────
//  TextureLoader::LoadHDR
// ─────────────────────────────────────────────────────────────────────────────
Texture2D TextureLoader::LoadHDR(const std::string& path)
{
    stbi_set_flip_vertically_on_load_thread(1);

    int w, h, ch;
    float* data = stbi_loadf(path.c_str(), &w, &h, &ch, 0);
    if (!data)
    {
        std::cerr << "[TextureLoader] Falha HDR: " << path << "\n";
        return {};
    }

    Texture2D tex;
    tex.Width    = w;
    tex.Height   = h;
    tex.Channels = ch;
    tex.Path     = path;

    glGenTextures(1, &tex.ID);
    glBindTexture(GL_TEXTURE_2D, tex.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0,
                 GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    std::cout << "[TextureLoader] HDR: " << path << " (" << w << "x" << h << ")\n";
    return tex;
}

// ─────────────────────────────────────────────────────────────────────────────
//  TextureLoader::LoadCubemap
// ─────────────────────────────────────────────────────────────────────────────
GLuint TextureLoader::LoadCubemap(const std::string paths[6])
{
    stbi_set_flip_vertically_on_load_thread(0);

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    for (int i = 0; i < 6; i++)
    {
        int w, h, ch;
        unsigned char* data = stbi_load(paths[i].c_str(), &w, &h, &ch, 0);
        if (!data)
        {
            std::cerr << "[TextureLoader] Cubemap face " << i
                      << " falhou: " << paths[i] << "\n";
            stbi_image_free(data);
            continue;
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8,
                     w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return id;
}