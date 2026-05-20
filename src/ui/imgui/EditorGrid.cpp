#include "EditorGrid.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
//  Shaders do grid (inline — não precisam de arquivo externo)
// ─────────────────────────────────────────────────────────────────────────────
static const char* s_GridVert = R"(
#version 460 core

// Quad fullscreen nos corners do clip space
const vec2 kPositions[6] = vec2[](
    vec2(-1, -1), vec2( 1, -1), vec2( 1,  1),
    vec2(-1, -1), vec2( 1,  1), vec2(-1,  1)
);

uniform mat4 uView;
uniform mat4 uProj;

out vec3 vNearPoint;
out vec3 vFarPoint;

vec3 UnprojectPoint(float x, float y, float z)
{
    mat4 invVP = inverse(uProj * uView);
    vec4 p = invVP * vec4(x, y, z, 1.0);
    return p.xyz / p.w;
}

void main()
{
    vec2 pos = kPositions[gl_VertexID];
    vNearPoint = UnprojectPoint(pos.x, pos.y, 0.0);
    vFarPoint  = UnprojectPoint(pos.x, pos.y, 1.0);
    gl_Position = vec4(pos, 0.0, 1.0);
}
)";

static const char* s_GridFrag = R"(
#version 460 core

in vec3 vNearPoint;
in vec3 vFarPoint;

uniform mat4  uView;
uniform mat4  uProj;
uniform vec3  uCamPos;
uniform vec2  uNearFar;   // x=near, y=far

out vec4 FragColor;

vec4 Grid(vec3 fragPos3D, float scale)
{
    vec2 coord = fragPos3D.xz * scale;
    vec2 deriv = fwidth(coord);
    vec2 grid  = abs(fract(coord - 0.5) - 0.5) / deriv;
    float line = min(grid.x, grid.y);
    float alpha = 1.0 - min(line, 1.0);

    float minX = min(deriv.x, 1.0);
    float minZ = min(deriv.y, 1.0);
    vec4 color = vec4(0.3, 0.3, 0.35, alpha * 0.6);

    if (fragPos3D.x > -minX * 0.5 && fragPos3D.x < minX * 0.5)
        color = vec4(0.85, 0.25, 0.25, 1.0);
    if (fragPos3D.z > -minZ * 0.5 && fragPos3D.z < minZ * 0.5)
        color = vec4(0.25, 0.40, 0.90, 1.0);

    return color;
}

void main()
{
    float t = -vNearPoint.y / (vFarPoint.y - vNearPoint.y);
    if (t < 0.0) discard;

    vec3 fragPos = vNearPoint + t * (vFarPoint - vNearPoint);

    // Depth correto a partir da posição world-space — sem isso a grid se sobrepõe à geometria
    vec4 clip = uProj * uView * vec4(fragPos, 1.0);
    gl_FragDepth = clamp((clip.z / clip.w) * 0.5 + 0.5, 0.0, 1.0);

    float dist  = length(fragPos.xz - uCamPos.xz);
    float fade  = 1.0 - smoothstep(uNearFar.y * 0.3, uNearFar.y * 0.9, dist);

    vec4 g1 = Grid(fragPos, 1.0);
    vec4 g2 = Grid(fragPos, 0.1);

    vec4 color = g1 + g2 * 0.6;
    color.a   *= fade;

    if (color.a < 0.01) discard;
    FragColor = color;
}
)";

// ─────────────────────────────────────────────────────────────────────────────
static GLuint CompileShader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        std::cerr << "[Grid Shader] " << log << "\n";
        glDeleteShader(s);
        return 0;
    }
    return s;
}

bool EditorGrid::Init()
{
    // VAO vazio — vertices gerados no vertex shader pelo gl_VertexID
    glGenVertexArrays(1, &m_VAO);

    // Compilar shader
    GLuint vert = CompileShader(GL_VERTEX_SHADER,   s_GridVert);
    GLuint frag = CompileShader(GL_FRAGMENT_SHADER, s_GridFrag);
    if (!vert || !frag) return false;

    m_Shader = glCreateProgram();
    glAttachShader(m_Shader, vert);
    glAttachShader(m_Shader, frag);
    glLinkProgram(m_Shader);
    glDeleteShader(vert);
    glDeleteShader(frag);

    GLint ok = 0;
    glGetProgramiv(m_Shader, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[512];
        glGetProgramInfoLog(m_Shader, 512, nullptr, log);
        std::cerr << "[Grid Program] " << log << "\n";
        return false;
    }

    m_LocView    = glGetUniformLocation(m_Shader, "uView");
    m_LocProj    = glGetUniformLocation(m_Shader, "uProj");
    m_LocCamPos  = glGetUniformLocation(m_Shader, "uCamPos");
    m_LocNearFar = glGetUniformLocation(m_Shader, "uNearFar");

    std::cout << "[EditorGrid] Inicializado.\n";
    return true;
}

void EditorGrid::Draw(const glm::mat4& view, const glm::mat4& proj,
                      const glm::vec3& camPos)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);          // grid não escreve no depth buffer

    glUseProgram(m_Shader);

    glUniformMatrix4fv(m_LocView, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(m_LocProj, 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(m_LocCamPos,  1, glm::value_ptr(camPos));
    glUniform2f(m_LocNearFar,  0.1f, 1000.0f);

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);   // quad = 2 triângulos = 6 vértices
    glBindVertexArray(0);

    glUseProgram(0);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void EditorGrid::Destroy()
{
    if (m_Shader) { glDeleteProgram(m_Shader);      m_Shader = 0; }
    if (m_VAO)    { glDeleteVertexArrays(1, &m_VAO); m_VAO   = 0; }
}