/**
 * @file SceneFrameBuffer.hpp
 * @brief Off-screen render target used by @ref EditorViewport and @ref GameViewport.
 *
 * Creates a framebuffer with one RGBA colour attachment and one depth/stencil
 * renderbuffer.  Call @ref Resize() whenever the viewport dimensions change.
 */
#pragma once
#include <glad/glad.h>

/// @brief Size specification passed to @ref SceneFramebuffer::Init().
struct FramebufferSpec
{
    int Width  = 1280; ///< Initial width in pixels.
    int Height = 720;  ///< Initial height in pixels.
};

/// @brief Off-screen framebuffer with a colour texture and a depth renderbuffer.
class SceneFramebuffer
{
public:
    SceneFramebuffer() = default;
    ~SceneFramebuffer() { Destroy(); }

    /**
     * @brief Allocates the framebuffer at the given size.
     * @param spec  Initial dimensions.
     */
    void Init(const FramebufferSpec& spec);

    /**
     * @brief Reallocates all attachments at the new dimensions.
     * @param width   New width in pixels.
     * @param height  New height in pixels.
     */
    void Resize(int width, int height);

    /// Frees all OpenGL objects.
    void Destroy();

    void Bind()   const; ///< Redirects subsequent draw calls to this framebuffer.
    void Unbind() const; ///< Restores the default framebuffer (screen).

    /// @return OpenGL texture ID of the colour attachment (use as an ImGui::Image() texture).
    GLuint GetColorAttachment() const { return m_ColorAttachment; }

    int GetWidth()  const { return m_Spec.Width;  } ///< @return Current framebuffer width.
    int GetHeight() const { return m_Spec.Height; } ///< @return Current framebuffer height.

private:
    void Rebuild();

    FramebufferSpec m_Spec;
    GLuint m_FBO             = 0;
    GLuint m_ColorAttachment = 0;
    GLuint m_DepthAttachment = 0;
};