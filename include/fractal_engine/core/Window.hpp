#pragma once

#include <string>

// Forward declaration para não expor GLFW no header
struct GLFWwindow;

struct WindowProps
{
    std::string Title  = "GameEngine";
    int         Width  = 1280;
    int         Height = 720;
    bool        VSync  = true;
};

class Window
{
public:
    Window()  = default;
    ~Window() = default;

    bool Init(const WindowProps& props);
    void Shutdown();

    void SwapBuffers();
    bool ShouldClose() const;

    // Getters
    int  GetWidth()  const { return m_Props.Width;  }
    int  GetHeight() const { return m_Props.Height; }
    bool IsVSync()   const { return m_Props.VSync;  }

    GLFWwindow* GetNativeWindow() const { return m_Window; }

    void SetVSync(bool enabled);

private:
    // Callbacks GLFW (estáticos pois o GLFW não conhece C++)
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void ErrorCallback(int error, const char* description);

private:
    GLFWwindow* m_Window = nullptr;
    WindowProps m_Props;
};