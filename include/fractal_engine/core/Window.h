#pragma once
#include <third_party/glad/glad.h>
#include <third_party/GLFW/glfw3.h>
#include "fractal_engine/input/Input.h"

namespace fractal_engine::core {

using fractal_engine::input::Input;

class Window
{
public:
    Window(int width, int height, const char* title);
    ~Window();

    bool init();
    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();

    GLFWwindow* getNativeWindow() const;
    Input& getInput() { return input; }

private:
    GLFWwindow* window;
    int         width;
    int         height;
    const char* title;
    Input       input;

    void centerWindow(GLFWmonitor* monitor);

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback           (GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback          (GLFWwindow* window, double xoffset, double yoffset); // ← novo
    static void key_callback             (GLFWwindow* window, int key, int scancode, int action, int mods);
};

} // namespace fractal_engine::core