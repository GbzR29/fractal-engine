#include "SkyRenderer.hpp"
#include "AssetLoader.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

SkyRenderer::~SkyRenderer()
{
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
}

bool SkyRenderer::Init(const std::string& shaderDir)
{
    const std::string vert = shaderDir + "/sky.vert";
    const std::string frag = shaderDir + "/sky.frag";

    if (!m_Shader.LoadFromFiles(vert, frag)) {
        std::cerr << "[SkyRenderer] Falha ao carregar sky shaders de: " << shaderDir << "\n";
        return false;
    }

    glGenVertexArrays(1, &m_VAO);

    m_Ready = true;
    std::cout << "[SkyRenderer] Sky shaders carregados.\n";
    return true;
}

void SkyRenderer::Draw(const glm::mat4& view, const glm::mat4& proj)
{
    if (!m_Ready) return;

    glm::mat4 invVP = glm::inverse(proj * view);

    // Desabilita depth write — sky está no fundo, não deve bloquear geometria
    GLboolean prevDepthMask;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &prevDepthMask);
    glDepthMask(GL_FALSE);

    m_Shader.Bind();
    m_Shader.SetMat4("uInvViewProj", invVP);

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    m_Shader.Unbind();

    // Restaura depth write
    glDepthMask(prevDepthMask);
}
