#include "Window.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

bool Window::Init(const WindowProps& props)
{
    m_Props = props;

    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
    {
        std::cerr << "[Window] Falha ao inicializar o GLFW.\n";
        return false;
    }

    // ── Hints OpenGL ────────────────────────────────────────────────────────
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // começa invisível para centralizar antes de exibir
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // ── Criar janela ────────────────────────────────────────────────────────
    m_Window = glfwCreateWindow(
        m_Props.Width, m_Props.Height,
        m_Props.Title.c_str(),
        nullptr, nullptr
    );

    if (!m_Window)
    {
        std::cerr << "[Window] Falha ao criar a janela GLFW.\n";
        glfwTerminate();
        return false;
    }

    // ── Centralizar na tela ──────────────────────────────────────────────────
    GLFWmonitor*       monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode    = glfwGetVideoMode(monitor);

    if (monitor && mode)
    {
        int posX = (mode->width  - m_Props.Width)  / 2;
        int posY = (mode->height - m_Props.Height) / 2;
        glfwSetWindowPos(m_Window, posX, posY);
    }

    glfwShowWindow(m_Window); // agora exibe já na posição certa

    glfwMakeContextCurrent(m_Window);

    // ── Carregar GLAD ───────────────────────────────────────────────────────
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cerr << "[Window] Falha ao inicializar o GLAD.\n";
        return false;
    }

    // ── Viewport & callbacks ─────────────────────────────────────────────────
    glViewport(0, 0, m_Props.Width, m_Props.Height);
    glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);

    SetVSync(m_Props.VSync);

    std::cout << "[Window] OpenGL " << glGetString(GL_VERSION) << "\n";
    return true;
}

void Window::Shutdown()
{
    if (m_Window)
    {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
}

void Window::SwapBuffers()
{
    glfwSwapBuffers(m_Window);
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(m_Window);
}

void Window::SetVSync(bool enabled)
{
    m_Props.VSync = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

// ── Callbacks ────────────────────────────────────────────────────────────────

void Window::FramebufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Window::ErrorCallback(int error, const char* description)
{
    std::cerr << "[GLFW Error " << error << "] " << description << "\n";
}