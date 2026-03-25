#pragma once
#include <glad/glad.h>

struct FramebufferSpec
{
    int Width  = 1280;
    int Height = 720;
};

class SceneFramebuffer
{
public:
    SceneFramebuffer() = default;
    ~SceneFramebuffer() { Destroy(); }

    void Init(const FramebufferSpec& spec);
    void Resize(int width, int height);
    void Destroy();

    void Bind()   const;
    void Unbind() const;

    GLuint GetColorAttachment() const { return m_ColorAttachment; }
    int    GetWidth()           const { return m_Spec.Width;      }
    int    GetHeight()          const { return m_Spec.Height;     }

private:
    void Rebuild();

    FramebufferSpec m_Spec;
    GLuint m_FBO              = 0;
    GLuint m_ColorAttachment  = 0;
    GLuint m_DepthAttachment  = 0;
};