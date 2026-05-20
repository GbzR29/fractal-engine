/**
 * @file Shader.hpp
 * @brief GLSL shader program wrapper with uniform location caching.
 *
 * Non-copyable (owns an OpenGL program ID), but movable.  Load from files with
 * @ref LoadFromFiles() or from in-memory source strings with @ref LoadFromSource().
 */
#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>

/// @brief Wrapper around an OpenGL shader program.
class Shader
{
public:
    Shader() = default;
    ~Shader() { Destroy(); }

    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& o) noexcept : m_ID(o.m_ID) { o.m_ID = 0; }
    Shader& operator=(Shader&& o) noexcept
    {
        if (this != &o) { Destroy(); m_ID = o.m_ID; o.m_ID = 0; }
        return *this;
    }

    // ── Loading ───────────────────────────────────────────────────────────────

    /**
     * @brief Compiles and links a shader program from source files on disk.
     * @param vertPath  Path to the vertex shader source file.
     * @param fragPath  Path to the fragment shader source file.
     * @param geomPath  Optional path to a geometry shader source file.
     * @return @c true on success; compilation errors are printed to stderr.
     */
    bool LoadFromFiles(const std::string& vertPath,
                       const std::string& fragPath,
                       const std::string& geomPath = "");

    /**
     * @brief Compiles and links a shader program from in-memory strings.
     *        Useful for inline shaders (e.g. the editor grid).
     * @param vertSrc  Null-terminated GLSL vertex source.
     * @param fragSrc  Null-terminated GLSL fragment source.
     * @param geomSrc  Optional null-terminated GLSL geometry source.
     * @return @c true on success.
     */
    bool LoadFromSource(const char* vertSrc,
                        const char* fragSrc,
                        const char* geomSrc = nullptr);

    /// Deletes the OpenGL program object.
    void Destroy();

    // ── Usage ─────────────────────────────────────────────────────────────────

    void   Bind()    const { glUseProgram(m_ID); } ///< Binds this program for subsequent draw calls.
    void   Unbind()  const { glUseProgram(0);    } ///< Unbinds any shader program.
    bool   IsValid() const { return m_ID != 0;   } ///< @return @c true if the program compiled successfully.
    GLuint GetID()   const { return m_ID;         } ///< @return The raw OpenGL program ID.

    // ── Uniform setters ───────────────────────────────────────────────────────

    void SetBool (const std::string& name, bool  v)  const; ///< Sets a @c bool uniform.
    void SetInt  (const std::string& name, int   v)  const; ///< Sets an @c int uniform.
    void SetFloat(const std::string& name, float v)  const; ///< Sets a @c float uniform.

    void SetVec2 (const std::string& name, const glm::vec2& v) const; ///< Sets a @c vec2 uniform.
    void SetVec3 (const std::string& name, const glm::vec3& v) const; ///< Sets a @c vec3 uniform.
    void SetVec4 (const std::string& name, const glm::vec4& v) const; ///< Sets a @c vec4 uniform.

    void SetMat3 (const std::string& name, const glm::mat3& m) const; ///< Sets a @c mat3 uniform.
    void SetMat4 (const std::string& name, const glm::mat4& m) const; ///< Sets a @c mat4 uniform.

    /**
     * @brief Sets an array of @c mat4 uniforms (used for skinning / instancing).
     * @param name   Base uniform name (array must be declared as @c mat4 name[count]).
     * @param data   Pointer to the first matrix.
     * @param count  Number of matrices to upload.
     */
    void SetMat4Array(const std::string& name,
                      const glm::mat4* data, int count) const;

private:
    GLint GetUniformLocation(const std::string& name) const;

    GLuint m_ID = 0;

    /// Cache avoids calling glGetUniformLocation() every frame.
    mutable std::unordered_map<std::string, GLint> m_LocationCache;
};