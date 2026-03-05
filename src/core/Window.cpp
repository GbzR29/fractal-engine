#include "fractal_engine/core/Window.h"
#include <iostream>

namespace fractal_engine::core {

/**
 * @brief Constructs a new Window instance.
 */
Window::Window(int width, int height, const char* title)
    : window(nullptr), width(width), height(height), title(title)
{
}

/**
 * @brief Destroys the window and terminates GLFW.
 */
Window::~Window()
{
    if (window)
        glfwDestroyWindow(window);

    glfwTerminate();
}

/**
 * @brief Initializes GLFW, GLAD, and the OpenGL context.
 */
bool Window::init()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    centerWindow(primaryMonitor);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);

    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return true;
}

/**
 * @brief Centers the window on the selected monitor.
 */
void Window::centerWindow(GLFWmonitor* monitor)
{
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    int monitorX, monitorY;
    glfwGetMonitorPos(monitor, &monitorX, &monitorY);

    int posX = monitorX + (mode->width - width) / 2;
    int posY = monitorY + (mode->height - height) / 2;

    glfwSetWindowPos(window, posX, posY);
}

/**
 * @brief Handles framebuffer resize events.
 */
void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

/**
 * @brief Handles mouse movement input.
 */
void Window::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    self->input.onMouseMove(xpos, ypos);
    std::cout << "Mouse move: " << xpos << " " << ypos << std::endl;
}

/**
 * @brief Handles keyboard input.
 */
void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

/**
 * @brief Checks whether the window should close.
 */
bool Window::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

/**
 * @brief Swaps rendering buffers.
 */
void Window::swapBuffers()
{
    glfwSwapBuffers(window);
}

/**
 * @brief Polls events from the system.
 */
void Window::pollEvents()
{
    glfwPollEvents();
}

/**
 * @brief Returns the native GLFW window.
 */
GLFWwindow* Window::getNativeWindow() const
{
    return window;
}

} // namespace fractal_engine::core