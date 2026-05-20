/**
 * @file TextureLoader.hpp
 * @brief Low-level OpenGL texture loading utilities (stb_image-based).
 *
 * @ref TextureLoader::Load() is the primary entry point for loading standard image files.
 * For PBR / asset-pipeline use, prefer the higher-level @ref Texture class which
 * supports async decode + GPU upload and caching via @ref AssetManager.
 */
#pragma once
#include <glad/glad.h>
#include <string>

/// @brief Parameters that control how a texture is created on the GPU.
struct TextureSpec
{
    bool   FlipY        = true;                    ///< Flip vertically (OpenGL expects bottom-left origin).
    bool   GenerateMips = true;                    ///< Generate full mipmap chain after upload.
    GLenum WrapS        = GL_REPEAT;               ///< Horizontal wrap mode.
    GLenum WrapT        = GL_REPEAT;               ///< Vertical wrap mode.
    GLenum MinFilter    = GL_LINEAR_MIPMAP_LINEAR; ///< Minification filter.
    GLenum MagFilter    = GL_LINEAR;               ///< Magnification filter.
    bool   sRGB         = false;                   ///< Use @c GL_SRGB8_ALPHA8 internal format for gamma-correct rendering.
};

/// @brief Lightweight POD wrapper around an OpenGL 2-D texture handle.
struct Texture2D
{
    GLuint      ID       = 0;  ///< OpenGL texture object ID.
    int         Width    = 0;  ///< Texture width in pixels.
    int         Height   = 0;  ///< Texture height in pixels.
    int         Channels = 0;  ///< Number of colour channels in the source image.
    std::string Path;          ///< Absolute path used to load this texture.

    bool IsValid() const { return ID != 0; } ///< @return @c true if the texture object is valid.

    /// Binds the texture to the given texture unit.
    void Bind(GLuint slot = 0) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, ID);
    }
    void Unbind()  const { glBindTexture(GL_TEXTURE_2D, 0); }
    void Destroy() { if (ID) { glDeleteTextures(1, &ID); ID = 0; } }
};

/**
 * @brief Creates a 1×1 solid colour texture.
 * @param r,g,b,a  RGBA components [0, 255].
 * @return A valid @ref Texture2D or an invalid one on failure.
 */
Texture2D CreateSolidTexture(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

Texture2D CreateWhiteTexture();         ///< 1×1 white RGBA texture (fallback albedo).
Texture2D CreateBlackTexture();         ///< 1×1 black RGBA texture (fallback metallic/AO).
Texture2D CreateDefaultNormalMap();     ///< 1×1 flat normal map: (128, 128, 255).

/// @brief Utility functions for loading image files directly to OpenGL textures.
namespace TextureLoader
{
    /**
     * @brief Loads a PNG/JPG/BMP/TGA/HDR image file and uploads it to the GPU.
     * @param path  Absolute path to the image file.
     * @param spec  Texture creation parameters.
     * @return A valid @ref Texture2D on success; @ref Texture2D::IsValid() is @c false on failure.
     */
    Texture2D Load(const std::string& path, const TextureSpec& spec = {});

    /**
     * @brief Loads an HDR image as a @c GL_RGB16F texture (used for IBL / skybox).
     * @param path  Absolute path to the HDR file.
     * @return Floating-point texture suitable for equirectangular projections.
     */
    Texture2D LoadHDR(const std::string& path);

    /**
     * @brief Loads a cubemap from six separate image files.
     * @param paths  Six paths in the order: +X, −X, +Y, −Y, +Z, −Z.
     * @return OpenGL cubemap texture ID, or 0 on failure.
     */
    GLuint LoadCubemap(const std::string paths[6]);
}