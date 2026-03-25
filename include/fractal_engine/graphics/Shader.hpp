#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>

class Shader
{
public:
    Shader() = default;
    ~Shader() { Destroy(); }

    // Sem cópia — recurso OpenGL
    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;

    // Move OK
    Shader(Shader&& o) noexcept : m_ID(o.m_ID) { o.m_ID = 0; }
    Shader& operator=(Shader&& o) noexcept
    {
        if (this != &o) { Destroy(); m_ID = o.m_ID; o.m_ID = 0; }
        return *this;
    }

    // ── Carregamento ──────────────────────────────────────────────────────────
    // A partir de arquivos no disco
    bool LoadFromFiles(const std::string& vertPath,
                       const std::string& fragPath,
                       const std::string& geomPath = "");

    // A partir de strings (útil para shaders inline como o grid)
    bool LoadFromSource(const char* vertSrc,
                        const char* fragSrc,
                        const char* geomSrc = nullptr);

    void Destroy();

    // ── Uso ───────────────────────────────────────────────────────────────────
    void Bind()   const { glUseProgram(m_ID); }
    void Unbind() const { glUseProgram(0);    }
    bool IsValid() const { return m_ID != 0;  }
    GLuint GetID() const { return m_ID;       }

    // ── Uniforms ──────────────────────────────────────────────────────────────
    void SetBool (const std::string& name, bool  v)  const;
    void SetInt  (const std::string& name, int   v)  const;
    void SetFloat(const std::string& name, float v)  const;

    void SetVec2 (const std::string& name, const glm::vec2& v) const;
    void SetVec3 (const std::string& name, const glm::vec3& v) const;
    void SetVec4 (const std::string& name, const glm::vec4& v) const;

    void SetMat3 (const std::string& name, const glm::mat3& m) const;
    void SetMat4 (const std::string& name, const glm::mat4& m) const;

    // Array de mat4 (skinning, instancing)
    void SetMat4Array(const std::string& name,
                      const glm::mat4* data, int count) const;

private:
    GLint GetUniformLocation(const std::string& name) const;

    GLuint m_ID = 0;

    // Cache de locations para evitar glGetUniformLocation todo frame
    mutable std::unordered_map<std::string, GLint> m_LocationCache;
};