#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

// Grid infinito renderizado com um quad fullscreen + shader
// Técnica: Ray-plane intersection no fragment shader
// Não precisa de VBO com linhas — escala infinitamente
class EditorGrid
{
public:
    EditorGrid()  = default;
    ~EditorGrid() { Destroy(); }

    bool Init();
    void Draw(const glm::mat4& view, const glm::mat4& proj,
              const glm::vec3& cameraPos);
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