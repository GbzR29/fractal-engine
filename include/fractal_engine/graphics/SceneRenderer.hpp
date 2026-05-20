/**
 * @file SceneRenderer.hpp
 * @brief Renders all scene entities with a PBR shader and draws selection outlines.
 *
 * Workflow:
 * 1. Call @ref Init() once after the OpenGL context is available.
 * 2. Each frame: @ref RenderEntities() → @ref RenderOutline() (optional).
 * 3. Set sun parameters with @ref SetSunDirection() / @ref SetSunColor() when the
 *    scene's directional light changes.
 */
#pragma once

#include "Shader.hpp"
#include "SceneEntity.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

/// @brief Draws all MeshRenderer entities in the scene using a PBR GLSL shader.
class SceneRenderer {
public:
    /**
     * @brief Compiles and links the PBR and outline shaders.
     * @param shaderDir  Directory that contains the @c .vert and @c .frag source files.
     * @return @c true on success.
     */
    bool Init(const std::string& shaderDir);

    /// @return @c true if the renderer is ready to draw.
    bool IsReady() const { return m_Ready; }

    /**
     * @brief Submits all active entities with a MeshRendererComponent for rendering.
     * @param entities    Scene entity list.
     * @param view        Camera view matrix.
     * @param projection  Camera projection matrix.
     * @param camPos      Camera world position (used for specular highlights).
     */
    void RenderEntities(
        const std::vector<std::unique_ptr<SceneEntity>>& entities,
        const glm::mat4& view,
        const glm::mat4& projection,
        const glm::vec3& camPos
    );

    /**
     * @brief Draws a backface-hull outline around a single entity (selection highlight).
     * @param entity      The selected entity.
     * @param view        Camera view matrix.
     * @param projection  Camera projection matrix.
     */
    void RenderOutline(
        const SceneEntity& entity,
        const glm::mat4& view,
        const glm::mat4& projection
    );

    /**
     * @brief Sets the directional light direction sent to the PBR shader.
     * @param dir  World-space direction vector (need not be normalised).
     */
    void SetSunDirection(const glm::vec3& dir)   { m_SunDir   = dir; }

    /**
     * @brief Sets the directional light colour sent to the PBR shader.
     * @param color  Linear-space HDR colour (values >1 are valid).
     */
    void SetSunColor(const glm::vec3& color)     { m_SunColor = color; }

private:
    void InitOutlineShader();

    Shader    m_Shader;
    Shader    m_OutlineShader;
    bool      m_Ready        = false;
    bool      m_OutlineReady = false;
    glm::vec3 m_SunDir       = glm::vec3(0.5f, 1.0f, 0.8f);
    glm::vec3 m_SunColor     = glm::vec3(3.0f, 3.0f, 2.8f);
};
