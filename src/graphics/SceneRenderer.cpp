#include "SceneRenderer.hpp"

#include "AssetLoader.hpp"
#include "Model.hpp"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
static const char* s_OutlineVert = R"(
#version 460 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uThickness;

void main()
{
    mat3 normalMat = transpose(inverse(mat3(uModel)));
    vec3 worldNormal = normalize(normalMat * aNormal);
    vec3 worldPos    = vec3(uModel * vec4(aPosition, 1.0)) + worldNormal * uThickness;
    gl_Position = uProjection * uView * vec4(worldPos, 1.0);
}
)";

static const char* s_OutlineFrag = R"(
#version 460 core
out vec4 fragColor;
uniform vec4 uColor;
void main() { fragColor = uColor; }
)";

void SceneRenderer::InitOutlineShader()
{
    if (m_OutlineShader.LoadFromSource(s_OutlineVert, s_OutlineFrag))
        m_OutlineReady = true;
    else
        std::cerr << "[SceneRenderer] Falha ao compilar outline shader.\n";
}

bool SceneRenderer::Init(const std::string& shaderDir)
{
    const std::string vert = shaderDir + "/pbr.vert";
    const std::string frag = shaderDir + "/pbr.frag";

    if (!m_Shader.LoadFromFiles(vert, frag)) {
        std::cerr << "[SceneRenderer] Falha ao carregar shaders PBR de: " << shaderDir << "\n";
        return false;
    }

    InitOutlineShader();

    m_Ready = true;
    std::cout << "[SceneRenderer] Shaders PBR carregados.\n";
    return true;
}

void SceneRenderer::RenderEntities(
    const std::vector<std::unique_ptr<SceneEntity>>& entities,
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& camPos)
{
    if (!m_Ready) return;

    m_Shader.Bind();
    m_Shader.SetMat4("uView",       view);
    m_Shader.SetMat4("uProjection", projection);
    m_Shader.SetVec3("uCamPos",     camPos);
    m_Shader.SetVec3("uLightDir",   glm::normalize(m_SunDir));
    m_Shader.SetVec3("uLightColor", m_SunColor);
    m_Shader.SetBool("uSkinned",    false);

    for (const auto& entity : entities) {
        if (!entity || !entity->Active || !entity->HasMeshRenderer()) continue;

        const auto& mr = entity->MeshRenderer;
        if (!mr->model) continue;

        m_Shader.SetMat4("uModel", entity->Transform.GetMatrix());

        if (mr->model->hasSkeleton() && mr->animator) {
            mr->animator->update(0.0f, *mr->model->skeleton); // update externo
            const auto& bones = mr->animator->getBoneTransforms();
            m_Shader.SetMat4Array("uBoneMatrices",
                                  bones.data(),
                                  static_cast<int>(bones.size()));
            m_Shader.SetBool("uSkinned", true);
        } else {
            m_Shader.SetBool("uSkinned", false);
        }

        mr->model->draw(m_Shader);
    }

    m_Shader.Unbind();
}

void SceneRenderer::RenderOutline(
    const SceneEntity& entity,
    const glm::mat4& view,
    const glm::mat4& projection)
{
    if (!m_OutlineReady) return;
    if (!entity.HasMeshRenderer() || !entity.MeshRenderer->model) return;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);   // mostra só as faces de trás (expandidas = outline)

    m_OutlineShader.Bind();
    m_OutlineShader.SetMat4("uView",       view);
    m_OutlineShader.SetMat4("uProjection", projection);
    m_OutlineShader.SetMat4("uModel",      entity.Transform.GetMatrix());
    m_OutlineShader.SetFloat("uThickness", 0.025f);
    m_OutlineShader.SetVec4("uColor",      glm::vec4(1.0f, 0.55f, 0.05f, 1.0f));

    entity.MeshRenderer->model->draw(m_OutlineShader);

    m_OutlineShader.Unbind();
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
}
