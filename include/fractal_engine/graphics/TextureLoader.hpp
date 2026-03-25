#pragma once
#include <glad/glad.h>
#include <string>

// Parâmetros de carregamento
struct TextureSpec
{
    bool  FlipY        = true;   // OpenGL espera origem no bottom-left
    bool  GenerateMips = true;
    GLenum WrapS       = GL_REPEAT;
    GLenum WrapT       = GL_REPEAT;
    GLenum MinFilter   = GL_LINEAR_MIPMAP_LINEAR;
    GLenum MagFilter   = GL_LINEAR;
    bool  sRGB         = false;  // ativa GL_SRGB8_ALPHA8 (gamma-correct)
};

// Resultado do carregamento
struct Texture2D
{
    GLuint ID      = 0;
    int    Width   = 0;
    int    Height  = 0;
    int    Channels= 0;
    std::string Path;

    bool IsValid() const { return ID != 0; }
    void Bind(GLuint slot = 0) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, ID);
    }
    void Unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }
    void Destroy()
    {
        if (ID) { glDeleteTextures(1, &ID); ID = 0; }
    }
};

// Textura de 1x1 pixel de cor sólida — útil como fallback
Texture2D CreateSolidTexture(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

// Texturas fallback padrão (branco, preto, normal map plana)
Texture2D CreateWhiteTexture();
Texture2D CreateBlackTexture();
Texture2D CreateDefaultNormalMap();  // (128, 128, 255) = normal apontando para cima

namespace TextureLoader
{
    // Carrega PNG, JPG, BMP, TGA, HDR...
    Texture2D Load(const std::string& path,
                   const TextureSpec& spec = {});

    // Carrega HDR como GL_RGB16F (para IBL / skybox)
    Texture2D LoadHDR(const std::string& path);

    // Cubemap: 6 arquivos na ordem +X -X +Y -Y +Z -Z
    GLuint LoadCubemap(const std::string paths[6]);
}