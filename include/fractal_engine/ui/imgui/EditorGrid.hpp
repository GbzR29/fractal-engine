/**
 * @file EditorGrid.hpp
 * @brief Infinite editor grid rendered via a fullscreen quad and a fragment shader.
 *
 * The grid is computed in the GLSL fragment shader using ray-plane intersection
 * against the Y=0 world plane.  No VBO of line segments is needed — the grid
 * scales smoothly regardless of zoom level.
 */
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

/// @brief Renders an infinite anti-aliased grid on the XZ plane (Y=0).
class EditorGrid
{
public:
    EditorGrid()  = default;
    ~EditorGrid() { Destroy(); }

    /**
     * @brief Compiles the grid shader and creates the fullscreen quad VAO.
     * @return @c true on success.
     */
    bool Init();

    /**
     * @brief Draws the grid for the current frame.
     * @param view       Camera view matrix.
     * @param proj       Camera projection matrix.
     * @param cameraPos  Camera world position (used for grid fade-out with distance).
     */
    void Draw(const glm::mat4& view, const glm::mat4& proj,
              const glm::vec3& cameraPos);

    /// Releases the VAO, VBO, and shader objects.
    void Destroy();

private:
    GLuint m_VAO     = 0;
    GLuint m_VBO     = 0;
    GLuint m_Shader  = 0;

    // Uniform locations
    GLint m_LocView      = -1;
    GLint m_LocProj      = -1;
    GLint m_LocCamPos    = -1;
    GLint m_LocNearFar   = -1;
};