#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers internos
// ─────────────────────────────────────────────────────────────────────────────
static std::string ReadFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[Shader] Arquivo não encontrado: " << path << "\n";
        return {};
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static GLuint CompileStage(GLenum type, const char* src, const std::string& label)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "[Shader] Erro em " << label << ":\n" << log << "\n";
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static bool LinkProgram(GLuint program)
{
    glLinkProgram(program);
    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);
        std::cerr << "[Shader] Erro de link:\n" << log << "\n";
        return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  LoadFromFiles
// ─────────────────────────────────────────────────────────────────────────────
bool Shader::LoadFromFiles(const std::string& vertPath,
                           const std::string& fragPath,
                           const std::string& geomPath)
{
    std::string vs = ReadFile(vertPath);
    std::string fs = ReadFile(fragPath);
    if (vs.empty() || fs.empty()) return false;

    const char* gs = nullptr;
    std::string gsStr;
    if (!geomPath.empty())
    {
        gsStr = ReadFile(geomPath);
        if (!gsStr.empty()) gs = gsStr.c_str();
    }

    return LoadFromSource(vs.c_str(), fs.c_str(), gs);
}

// ─────────────────────────────────────────────────────────────────────────────
//  LoadFromSource
// ─────────────────────────────────────────────────────────────────────────────
bool Shader::LoadFromSource(const char* vertSrc,
                            const char* fragSrc,
                            const char* geomSrc)
{
    Destroy();
    m_LocationCache.clear();

    GLuint vert = CompileStage(GL_VERTEX_SHADER,   vertSrc, "vertex");
    GLuint frag = CompileStage(GL_FRAGMENT_SHADER, fragSrc, "fragment");
    if (!vert || !frag)
    {
        glDeleteShader(vert);
        glDeleteShader(frag);
        return false;
    }

    GLuint geom = 0;
    if (geomSrc)
    {
        geom = CompileStage(GL_GEOMETRY_SHADER, geomSrc, "geometry");
        if (!geom)
        {
            glDeleteShader(vert);
            glDeleteShader(frag);
            return false;
        }
    }

    m_ID = glCreateProgram();
    glAttachShader(m_ID, vert);
    glAttachShader(m_ID, frag);
    if (geom) glAttachShader(m_ID, geom);

    bool ok = LinkProgram(m_ID);

    glDetachShader(m_ID, vert);
    glDetachShader(m_ID, frag);
    if (geom) glDetachShader(m_ID, geom);

    glDeleteShader(vert);
    glDeleteShader(frag);
    if (geom) glDeleteShader(geom);

    if (!ok) { Destroy(); return false; }

    std::cout << "[Shader] Compilado OK (ID=" << m_ID << ")\n";
    return true;
}

void Shader::Destroy()
{
    if (m_ID) { glDeleteProgram(m_ID); m_ID = 0; }
    m_LocationCache.clear();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Uniforms
// ─────────────────────────────────────────────────────────────────────────────
GLint Shader::GetUniformLocation(const std::string& name) const
{
    auto it = m_LocationCache.find(name);
    if (it != m_LocationCache.end()) return it->second;

    GLint loc = glGetUniformLocation(m_ID, name.c_str());
    if (loc == -1)
        std::cerr << "[Shader] Uniform não encontrado: " << name << "\n";

    m_LocationCache[name] = loc;
    return loc;
}

void Shader::SetBool (const std::string& n, bool  v)  const { glUniform1i (GetUniformLocation(n), (int)v); }
void Shader::SetInt  (const std::string& n, int   v)  const { glUniform1i (GetUniformLocation(n), v);      }
void Shader::SetFloat(const std::string& n, float v)  const { glUniform1f (GetUniformLocation(n), v);      }

void Shader::SetVec2(const std::string& n, const glm::vec2& v) const
{ glUniform2fv(GetUniformLocation(n), 1, glm::value_ptr(v)); }

void Shader::SetVec3(const std::string& n, const glm::vec3& v) const
{ glUniform3fv(GetUniformLocation(n), 1, glm::value_ptr(v)); }

void Shader::SetVec4(const std::string& n, const glm::vec4& v) const
{ glUniform4fv(GetUniformLocation(n), 1, glm::value_ptr(v)); }

void Shader::SetMat3(const std::string& n, const glm::mat3& m) const
{ glUniformMatrix3fv(GetUniformLocation(n), 1, GL_FALSE, glm::value_ptr(m)); }

void Shader::SetMat4(const std::string& n, const glm::mat4& m) const
{ glUniformMatrix4fv(GetUniformLocation(n), 1, GL_FALSE, glm::value_ptr(m)); }

void Shader::SetMat4Array(const std::string& n,
                          const glm::mat4* data, int count) const
{ glUniformMatrix4fv(GetUniformLocation(n), count, GL_FALSE,
                     glm::value_ptr(data[0])); }