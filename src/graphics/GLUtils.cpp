#include "GLUtils.hpp"
#include <iostream>

const char* GLErrorToString(GLenum error)
{
    switch (error)
    {
        case GL_NO_ERROR:                      return "GL_NO_ERROR";
        case GL_INVALID_ENUM:                  return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:                 return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:             return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY:                 return "GL_OUT_OF_MEMORY";
        default:                               return "GL_UNKNOWN_ERROR";
    }
}

bool _GLCheckError(const char* file, int line)
{
    GLenum err;
    bool ok = true;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "[OpenGL] " << GLErrorToString(err)
                  << " — " << file << ":" << line << "\n";
        ok = false;
    }
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Debug callback (OpenGL 4.3+)
// ─────────────────────────────────────────────────────────────────────────────
static void APIENTRY DebugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei /*length*/, const GLchar* message, const void* /*userParam*/)
{
    // Filtra mensagens triviais
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    const char* srcStr   = "?";
    const char* typeStr  = "?";
    const char* sevStr   = "?";

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             srcStr  = "API";             break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: srcStr  = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     srcStr  = "Application";     break;
        default: break;
    }
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               typeStr = "Error";        break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated";   break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance";  break;
        default: break;
    }
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:   sevStr = "HIGH";   break;
        case GL_DEBUG_SEVERITY_MEDIUM: sevStr = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW:    sevStr = "LOW";    break;
        default:                       sevStr = "NOTIFY"; break;
    }

    std::cerr << "[GL Debug] [" << sevStr << "] [" << typeStr << "] "
              << srcStr << " — " << message << "\n";
}

void GLEnableDebugOutput()
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(DebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
                          GL_DEBUG_SEVERITY_NOTIFICATION,
                          0, nullptr, GL_FALSE); // silencia notificações
    std::cout << "[GL] Debug output ativo.\n";
}

// ─────────────────────────────────────────────────────────────────────────────
//  GL namespace
// ─────────────────────────────────────────────────────────────────────────────
std::string GL::GetVendor()
{
    return reinterpret_cast<const char*>(glGetString(GL_VENDOR));
}
std::string GL::GetRenderer()
{
    return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}
std::string GL::GetVersion()
{
    return reinterpret_cast<const char*>(glGetString(GL_VERSION));
}
std::string GL::GetGLSLVersion()
{
    return reinterpret_cast<const char*>(
        glGetString(GL_SHADING_LANGUAGE_VERSION));
}