#include "fractal_engine/graphics/TextureLoader.h"
#include <iostream>
#include <sstream>

// stb_image deve ser definido UMA VEZ em todo o projeto.
// Se já foi definido em outro .cpp, remova a linha abaixo.
#define STB_IMAGE_IMPLEMENTATION
#include <third_party/stb_image/stb_image.h>

namespace fractal_engine::graphics {

// ─────────────────────────────────────────────────────────────────────────────
// Definição do cache estático
// ─────────────────────────────────────────────────────────────────────────────
std::unordered_map<std::string, GLuint> TextureLoader::s_cache;

// ─────────────────────────────────────────────────────────────────────────────
// Logger interno
// ─────────────────────────────────────────────────────────────────────────────
namespace {
    void logInfo (const std::string& msg) { std::cout << "[TextureLoader]        " << msg << "\n"; }
    void logWarn (const std::string& msg) { std::clog << "[TextureLoader][WARN]  " << msg << "\n"; }
    void logError(const std::string& msg) { std::cerr << "[TextureLoader][ERROR] " << msg << "\n"; }
}

// ─────────────────────────────────────────────────────────────────────────────
// Core: carrega uma imagem e envia para um texID já gerado
// ─────────────────────────────────────────────────────────────────────────────
void TextureLoader::loadInto(GLuint texID, const char* path) {
    glBindTexture(GL_TEXTURE_2D, texID);

    // Wrapping e filtragem estilo voxel/pixel-art
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_set_flip_vertically_on_load(true);

    int width = 0, height = 0, channels = 0;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

    if (!data) {
        logError("Failed to load \"" + std::string(path) + "\" — " + stbi_failure_reason());

        // Textura magenta 1x1 como fallback visual (fácil de identificar)
        unsigned char fallback[] = { 255, 0, 255, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, fallback);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }

    if (channels != 3 && channels != 4) {
        logWarn("\"" + std::string(path) + "\" has " + std::to_string(channels) +
                " channels — expected 3 (RGB) or 4 (RGBA). Visual artifacts may occur.");
    }

    GLenum format         = (channels == 4) ? GL_RGBA  : GL_RGB;
    GLenum internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    std::ostringstream oss;
    oss << "Loaded  \"" << path << "\"  "
        << width << "x" << height << "  "
        << channels << "ch  id=" << texID;
    logInfo(oss.str());

    glBindTexture(GL_TEXTURE_2D, 0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Interface original — mantida para compatibilidade com Chunk.cpp
// Uso: TextureLoader(textures[0], "assets/grass_top.png");
// ─────────────────────────────────────────────────────────────────────────────
TextureLoader::TextureLoader(GLuint texID, const char* path) {
    loadInto(texID, path);
}

// ─────────────────────────────────────────────────────────────────────────────
// Interface com cache — recomendada para novos sistemas
// Uso: GLuint id = TextureLoader::load("assets/stone.png");
// ─────────────────────────────────────────────────────────────────────────────
GLuint TextureLoader::load(const char* path) {
    std::string key(path);

    auto it = s_cache.find(key);
    if (it != s_cache.end()) {
        logInfo("Cache hit  \"" + key + "\"  id=" + std::to_string(it->second));
        return it->second;
    }

    GLuint texID = 0;
    glGenTextures(1, &texID);
    loadInto(texID, path);
    s_cache[key] = texID;
    return texID;
}

// ─────────────────────────────────────────────────────────────────────────────
// Limpa o cache e deleta as texturas da GPU
// Chamar em shutdown: TextureLoader::clearCache();
// ─────────────────────────────────────────────────────────────────────────────
void TextureLoader::clearCache() {
    for (auto& [path, id] : s_cache) {
        glDeleteTextures(1, &id);
        logInfo("Liberada  \"" + path + "\"  id=" + std::to_string(id));
    }
    s_cache.clear();
}
} // namespace fractal_engine::graphics