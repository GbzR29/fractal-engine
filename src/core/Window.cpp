#include "fractal_engine/core/Window.h"
#include <iostream>

namespace fractal_engine::core {

Window::Window(int width, int height, const char* title)
    : window(nullptr), width(width), height(height), title(title)
{}

Window::~Window()
{
    if (window) glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::init()
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    centerWindow(glfwGetPrimaryMonitor());
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    glEnable(GL_DEPTH_TEST);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback      (window, mouse_callback);
    glfwSetScrollCallback         (window, scroll_callback);           // ← registra
    glfwSetKeyCallback            (window, key_callback);
    glfwSetInputMode              (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return true;
}

void Window::centerWindow(GLFWmonitor* monitor)
{
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    int mx, my;
    glfwGetMonitorPos(monitor, &mx, &my);
    glfwSetWindowPos(window,
        mx + (mode->width  - width)  / 2,
        my + (mode->height - height) / 2);
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Window::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    self->input.onMouseMove(xpos, ypos);
    // FIX: removido o std::cout de debug — gerava output massivo todo frame
}

// ── Novo ──────────────────────────────────────────────────────────────────────
void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    self->input.onScroll(xoffset, yoffset);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

bool Window::shouldClose() const { return glfwWindowShouldClose(window); }
void Window::swapBuffers()       { glfwSwapBuffers(window); }
void Window::pollEvents()        { glfwPollEvents(); }
GLFWwindow* Window::getNativeWindow() const { return window; }

} // namespace fractal_engine::core