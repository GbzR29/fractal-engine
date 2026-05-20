/**
 * @file GLUtils.hpp
 * @brief OpenGL utility helpers: error checking, VAO/VBO/UBO wrappers, and state shortcuts.
 *
 * Include this header instead of including OpenGL headers directly in subsystem code.
 * Debug checks are compiled out in Release builds — no runtime overhead in production.
 */
#pragma once
#include <glad/glad.h>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  Error checking
// ─────────────────────────────────────────────────────────────────────────────

/// @return A human-readable string for an OpenGL error code (e.g. "GL_INVALID_ENUM").
const char* GLErrorToString(GLenum error);

/** @brief Checks for pending OpenGL errors and prints them to @c stderr with file/line context.
 *  @return @c false if any error was found.
 *  Prefer the @ref GL_CHECK macro over calling this function directly.
 */
bool _GLCheckError(const char* file, int line);

/** @def GL_CHECK(call)
 *  Wraps an OpenGL call and asserts no error occurred (debug builds only).
 *  @code GL_CHECK(glBindVertexArray(vao)); @endcode
 */
#ifdef _DEBUG
    #define GL_CHECK(call)    call; _GLCheckError(__FILE__, __LINE__)
    #define GL_CHECK_ERRORS() _GLCheckError(__FILE__, __LINE__)
#else
    #define GL_CHECK(call)    call
    #define GL_CHECK_ERRORS() (void)0
#endif

/// Registers @c glDebugMessageCallback for OpenGL 4.3+ debug output.  Call once after context creation.
void GLEnableDebugOutput();

// ─────────────────────────────────────────────────────────────────────────────
//  VAO / VBO helpers
// ─────────────────────────────────────────────────────────────────────────────
struct VAO
{
    GLuint ID = 0;

    void Create()  { glGenVertexArrays(1, &ID); }
    void Bind()    const { glBindVertexArray(ID); }
    void Unbind()  const { glBindVertexArray(0);  }
    void Destroy() { if (ID) { glDeleteVertexArrays(1, &ID); ID = 0; } }
};

struct VBO
{
    GLuint ID     = 0;
    GLenum Target = GL_ARRAY_BUFFER;

    void Create(GLenum target = GL_ARRAY_BUFFER) { Target = target; glGenBuffers(1, &ID); }
    void Bind()   const { glBindBuffer(Target, ID); }
    void Unbind() const { glBindBuffer(Target, 0);  }

    // Carrega dados estáticos
    void Upload(const void* data, GLsizeiptr size,
                GLenum usage = GL_STATIC_DRAW) const
    {
        Bind();
        glBufferData(Target, size, data, usage);
    }

    // Atualiza subregião (buffer dinâmico)
    void UpdateSub(const void* data, GLsizeiptr size,
                   GLintptr offset = 0) const
    {
        Bind();
        glBufferSubData(Target, offset, size, data);
    }

    void Destroy() { if (ID) { glDeleteBuffers(1, &ID); ID = 0; } }
};

// ─────────────────────────────────────────────────────────────────────────────
//  Vertex Attribute helpers
// ─────────────────────────────────────────────────────────────────────────────

// Configura um atributo float (o mais comum: posição, UV, normal...)
inline void VertexAttribF(GLuint index, GLint size,
                          GLsizei stride, GLsizeiptr offset,
                          bool normalized = false)
{
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, GL_FLOAT, normalized,
                          stride, (const void*)offset);
}

// Configura um atributo inteiro (indices de bone, IDs...)
inline void VertexAttribI(GLuint index, GLint size,
                          GLsizei stride, GLsizeiptr offset)
{
    glEnableVertexAttribArray(index);
    glVertexAttribIPointer(index, size, GL_INT,
                           stride, (const void*)offset);
}

// Divisor para instancing
inline void VertexAttribDivisor(GLuint index, GLuint divisor)
{
    glVertexAttribDivisor(index, divisor);
}

// ─────────────────────────────────────────────────────────────────────────────
//  UBO (Uniform Buffer Object)
//  Uso: UBO ubo; ubo.Create(sizeof(MyData), 0); ubo.Upload(&data, sizeof(data));
// ─────────────────────────────────────────────────────────────────────────────
struct UBO
{
    GLuint ID      = 0;
    GLuint Binding = 0;

    void Create(GLsizeiptr size, GLuint bindingPoint,
                GLenum usage = GL_DYNAMIC_DRAW)
    {
        Binding = bindingPoint;
        glGenBuffers(1, &ID);
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
        glBufferData(GL_UNIFORM_BUFFER, size, nullptr, usage);
        glBindBufferBase(GL_UNIFORM_BUFFER, Binding, ID);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void Upload(const void* data, GLsizeiptr size,
                GLintptr offset = 0) const
    {
        glBindBuffer(GL_UNIFORM_BUFFER, ID);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    // Liga o shader ao binding point
    void BindShader(GLuint shaderProgram, const char* blockName) const
    {
        GLuint idx = glGetUniformBlockIndex(shaderProgram, blockName);
        if (idx != GL_INVALID_INDEX)
            glUniformBlockBinding(shaderProgram, idx, Binding);
    }

    void Destroy() { if (ID) { glDeleteBuffers(1, &ID); ID = 0; } }
};

// ─────────────────────────────────────────────────────────────────────────────
//  Estado OpenGL — wrappers legíveis
// ─────────────────────────────────────────────────────────────────────────────
namespace GL
{
    inline void EnableDepthTest()    { glEnable(GL_DEPTH_TEST);    }
    inline void DisableDepthTest()   { glDisable(GL_DEPTH_TEST);   }
    inline void EnableBlend()        { glEnable(GL_BLEND);         }
    inline void DisableBlend()       { glDisable(GL_BLEND);        }
    inline void EnableCullFace()     { glEnable(GL_CULL_FACE);     }
    inline void DisableCullFace()    { glDisable(GL_CULL_FACE);    }
    inline void EnableWireframe()    { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
    inline void DisableWireframe()   { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

    inline void BlendAlpha()
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
    }

    inline void BlendAdditive()
    {
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
    }

    inline void SetViewport(int x, int y, int w, int h)
    {
        glViewport(x, y, w, h);
    }

    inline void Clear(float r = 0.0f, float g = 0.0f,
                      float b = 0.0f, float a = 1.0f)
    {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // Informações do driver — útil para debug/log na inicialização
    std::string GetVendor();
    std::string GetRenderer();
    std::string GetVersion();
    std::string GetGLSLVersion();
}