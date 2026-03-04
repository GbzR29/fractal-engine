#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../input/Input.h"

/**
 * @class Window
 * @brief Responsible for creating and managing the application window and OpenGL context.
 *
 * This class encapsulates GLFW window creation, OpenGL initialization using GLAD,
 * input callback binding, and window lifecycle management.
 */
class Window
{
public:
    /**
     * @brief Constructs a new Window object.
     *
     * @param width Width of the window.
     * @param height Height of the window.
     * @param title Title of the window.
     */
    Window(int width, int height, const char* title);

    /**
     * @brief Destroys the window and terminates GLFW.
     */
    ~Window();

    /**
     * @brief Initializes GLFW, OpenGL context, and callbacks.
     *
     * Must be called before using the window.
     *
     * @return true if initialization succeeded.
     * @return false if initialization failed.
     */
    bool init();

    /**
     * @brief Checks if the window should close.
     *
     * @return true if the window is requested to close.
     * @return false otherwise.
     */
    bool shouldClose() const;

    /**
     * @brief Swaps the front and back buffers.
     */
    void swapBuffers();

    /**
     * @brief Polls window events.
     */
    void pollEvents();

    /**
     * @brief Returns the internal GLFW window pointer.
     *
     * @return GLFWwindow*
     */
    GLFWwindow* getNativeWindow() const;


    Input& getInput() { return input; }

private:
    GLFWwindow* window;
    int width;
    int height;
    const char* title;

    Input input;

private:
    /**
     * @brief Centers the window on the given monitor.
     *
     * @param monitor Monitor where the window will be centered.
     */
    void centerWindow(GLFWmonitor* monitor);

    /**
     * @brief Framebuffer resize callback.
     */
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    /**
     * @brief Mouse movement callback.
     */
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

    /**
     * @brief Keyboard input callback.
     */
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

#endif