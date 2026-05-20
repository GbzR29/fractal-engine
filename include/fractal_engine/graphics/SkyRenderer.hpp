/**
 * @file SkyRenderer.hpp
 * @brief Procedural sky renderer — draws a gradient sky dome using a fullscreen pass.
 *
 * Call @ref Init() once after the OpenGL context is ready, then @ref Draw() each frame
 * before rendering any scene geometry (depth test should be disabled or set to @c GL_LEQUAL).
 */
#pragma once
#include "Shader.hpp"
#include <glm/glm.hpp>
#include <string>

/// @brief Renders a procedural sky background using a GLSL sky shader.
class SkyRenderer
{
public:
    ~SkyRenderer();

    /**
     * @brief Compiles the sky shader and creates the fullscreen quad VAO.
     * @param shaderDir  Directory containing the sky vertex and fragment shader files.
     * @return @c true on success.
     */
    bool Init(const std::string& shaderDir);

    /**
     * @brief Renders the sky for the current frame.
     * @param view  Camera view matrix (translation is stripped to keep the sky at infinity).
     * @param proj  Camera projection matrix.
     */
    void Draw(const glm::mat4& view, const glm::mat4& proj);

private:
    Shader   m_Shader;
    unsigned m_VAO   = 0;
    bool     m_Ready = false;
};
