#include "SceneFramebuffer.hpp"
#include <iostream>
#include <cassert>

void SceneFramebuffer::Init(const FramebufferSpec& spec)
{
    m_Spec = spec;
    Rebuild();
}

void SceneFramebuffer::Rebuild()
{
    // Destrói se já existir (resize)
    if (m_FBO)
        Destroy();

    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    // ── Color attachment (textura que o ImGui vai exibir) ─────────────────────
    glGenTextures(1, &m_ColorAttachment);
    glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 m_Spec.Width, m_Spec.Height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_ColorAttachment, 0);

    // ── Depth + stencil attachment (RenderBuffer — não precisa ser lido) ──────
    glGenRenderbuffers(1, &m_DepthAttachment);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                          m_Spec.Width, m_Spec.Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, m_DepthAttachment);

    // ── Verifica integridade ──────────────────────────────────────────────────
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[Framebuffer] Incompleto!\n";
    else
        std::cout << "[Framebuffer] " << m_Spec.Width << "x" << m_Spec.Height << " OK\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneFramebuffer::Resize(int width, int height)
{
    if (width == m_Spec.Width && height == m_Spec.Height) return;
    if (width  <= 0) width  = 1;
    if (height <= 0) height = 1;

    m_Spec.Width  = width;
    m_Spec.Height = height;
    Rebuild();
}

void SceneFramebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glViewport(0, 0, m_Spec.Width, m_Spec.Height);
}

void SceneFramebuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneFramebuffer::Destroy()
{
    if (m_ColorAttachment) { glDeleteTextures(1, &m_ColorAttachment);      m_ColorAttachment = 0; }
    if (m_DepthAttachment) { glDeleteRenderbuffers(1, &m_DepthAttachment); m_DepthAttachment = 0; }
    if (m_FBO)             { glDeleteFramebuffers(1, &m_FBO);              m_FBO             = 0; }
}