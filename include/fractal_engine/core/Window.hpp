/**
 * @file Window.hpp
 * @brief GLFW window wrapper — handles creation, OpenGL context, and VSync.
 */
#pragma once

#include <string>

struct GLFWwindow; ///< Forward declaration — keeps GLFW out of downstream headers.

/// @brief Configuration passed to @ref Window::Init().
struct WindowProps
{
    std::string Title  = "GameEngine"; ///< Title bar text.
    int         Width  = 1280;         ///< Initial width in pixels.
    int         Height = 720;          ///< Initial height in pixels.
    bool        VSync  = true;         ///< Enable vertical synchronisation.
};

/// @brief Thin wrapper around a GLFW window with an OpenGL 4.6 Core Profile context.
class Window
{
public:
    Window()  = default;
    ~Window() = default;

    /**
     * @brief Creates the GLFW window and initialises the OpenGL context.
     * @param props  Initial window properties.
     * @return @c true on success.
     */
    bool Init(const WindowProps& props);

    /// Destroys the window and terminates GLFW.
    void Shutdown();

    /// Swaps the front and back buffers (call once per frame).
    void SwapBuffers();

    /// @return @c true when the user has requested the window to close.
    bool ShouldClose() const;

    /// @return Current framebuffer width in pixels.
    int  GetWidth()  const { return m_Props.Width;  }

    /// @return Current framebuffer height in pixels.
    int  GetHeight() const { return m_Props.Height; }

    /// @return @c true if VSync is currently enabled.
    bool IsVSync()   const { return m_Props.VSync;  }

    /// @return The underlying @c GLFWwindow pointer (needed for GLFW callbacks and ImGui).
    GLFWwindow* GetNativeWindow() const { return m_Window; }

    /**
     * @brief Enables or disables vertical synchronisation.
     * @param enabled  Pass @c true to enable VSync.
     */
    void SetVSync(bool enabled);

private:
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void ErrorCallback(int error, const char* description);

private:
    GLFWwindow* m_Window = nullptr;
    WindowProps m_Props;
};